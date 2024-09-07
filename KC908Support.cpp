#include <SoapySDR/Device.hpp>
#include <SoapySDR/Registry.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Logger.h>
#include <SoapySDR/Types.h>
#include <inttypes.h>
#include"kcsdr.h"
#include <fstream>

/***********************************************************************
 * Device interface
 **********************************************************************/
class KC908 : public SoapySDR::Device
{
    public:
        sdr_api* sdr_handler;
        sdr_obj* sdr;
        std::ofstream out_file;

        bool running = false;
        double rx_bw = 40000000;
        double tx_bw = 40000000;

        double rx_freq = 100000000;
        double tx_freq = 100000000;

        double rx_bw_max;
        double rx_bw_min;

        double tx_bw_max;
        double tx_bw_min;

        double rx_freq_max;
        double rx_freq_min;

        double tx_freq_max;
        double tx_freq_min;

        uint16_t* d_buf = new uint16_t[2 * 409600];

        KC908(const SoapySDR::Kwargs &args)
        {
            (void) args;

            SoapySDR_logf(SOAPY_SDR_WARNING, "Constructor");

            sdr_handler = kcsdr_init();

            sdr = sdr_handler->find(KC_908_1);

            rx_freq_min = sdr->port[0].rx_freq.minimum;
            rx_freq_max = sdr->port[0].rx_freq.maximum;

            tx_freq_min = sdr->port[1].tx_freq.minimum;
            tx_freq_max = sdr->port[1].tx_freq.maximum;

            rx_bw_min = sdr->port[0].samp_rate.minimum;
            rx_bw_max = sdr->port[0].samp_rate.maximum;

            tx_bw_min = sdr->port[1].samp_rate.minimum;
            tx_bw_max = sdr->port[1].samp_rate.maximum;

            sdr_handler->close(sdr);
        }

        /*******************************************************************
        * Channels API
        ******************************************************************/

        size_t getNumChannels(const int dir) const
        {
            SoapySDR_logf(SOAPY_SDR_WARNING, "Get num channels");
            return 1;
        }

        bool getFullDuplex(const int direction, const size_t channel) const
        {
            SoapySDR_logf(SOAPY_SDR_WARNING, "Get full duplex");
            return true;
        }

        /*******************************************************************
        * Stream API
        ******************************************************************/

        std::vector<std::string> getStreamFormats(const int direction, const size_t channel) const {
            SoapySDR_logf(SOAPY_SDR_WARNING, "Get stream formats");
            std::vector<std::string> formats;

            formats.push_back(SOAPY_SDR_CF32);

            return formats;
        }

        std::string getNativeStreamFormat(const int direction, const size_t channel, double &fullScale) const
        {
            SoapySDR_logf(SOAPY_SDR_WARNING, "Get native format");
            return SOAPY_SDR_CF32;
        }

        SoapySDR::Stream *setupStream(
            const int direction,
            const std::string &format,
            const std::vector<size_t> &channels = std::vector<size_t>(),
            const SoapySDR::Kwargs &args = SoapySDR::Kwargs())
        {
            SoapySDR_logf(SOAPY_SDR_WARNING, "setup stream");
            SoapySDR_logf(SOAPY_SDR_WARNING, "%s", format.c_str());

            if(direction == SOAPY_SDR_RX) {
                return RX_STREAM;
            } else {
                return TX_STREAM;
            }
        }

        void closeStream(SoapySDR::Stream *stream)
        {
            SoapySDR_logf(SOAPY_SDR_WARNING, "Close stream");
            this->deactivateStream(stream, 0, 0);
        }

        // TODO: support flags

        virtual int activateStream(
            SoapySDR::Stream *stream,
            const int flags = 0,
            const long long timeNs = 0,
            const size_t numElems = 0)
        {
            SoapySDR_logf(SOAPY_SDR_WARNING, "Activate stream");
            if(stream == RX_STREAM){
                SoapySDR_logf(SOAPY_SDR_WARNING, "Activating RX");

                sdr = sdr_handler->find(KC_908_1);

                sdr_handler->rx_bw(sdr, rx_bw);
                sdr_handler->rx_freq(sdr, rx_freq);

                sdr_handler->rx_ext_amp(sdr, 0);
                sdr_handler->rx_amp(sdr, 20);
                sdr_handler->rx_att(sdr, 0);
                sdr_handler->rx_start(sdr);
            } else {
                sdr = sdr_handler->find(KC_908_1);

                sdr_handler->tx_bw(sdr, tx_bw);

                sdr_handler->tx_start(sdr);
            }

            running = true;

            return 0;
        }

        virtual int deactivateStream(
            SoapySDR::Stream *stream,
            const int flags = 0,
            const long long timeNs = 0)
        {
            SoapySDR_logf(SOAPY_SDR_WARNING, "Deactivate stream");
            if(running == false) {
                return 0;
            }

            running = false;

            if(stream == RX_STREAM){
                sdr_handler->rx_stop(sdr);
                sdr_handler->close(sdr);
            } else {
                sdr_handler->tx_stop(sdr);
                sdr_handler->close(sdr);
            }

            SoapySDR_logf(SOAPY_SDR_WARNING, "Stopped");

            return 0;
        }

        void u16_to_f32(const uint16_t* in, float* out, int count) {
            for (int i = 0; i < count; i++) {
                // TODO: Samples are using all of the resolution. Figure out how to properly scalee it, 32f is temporary solution
                float new_out = (float)(*in++) * (1.0f / 32768.0f / 32.f) - 1.f;

                *out++ = new_out;
            }
        }

        virtual int readStream(
            SoapySDR::Stream *stream,
            void * const *buffs,
            const size_t numElems,
            int &flags,
            long long &timeNs,
            const long timeoutUs = 100000)
        {
            SoapySDR_logf(SOAPY_SDR_WARNING, "Read stream");
            if(stream != RX_STREAM){
                return SOAPY_SDR_NOT_SUPPORTED;
            }

            bool ret = false;

            do {
                if(!running) {
                    return -1;
                }

                ret = sdr_handler->read(sdr, (uint8_t *)d_buf, numElems * 2 * sizeof(uint16_t));
            } while(ret == false);

            u16_to_f32(d_buf, (float*)buffs[0], numElems * 2);

            return numElems;
        }

        /*******************************************************************
        * Gain API
        ******************************************************************/

        std::vector<std::string> listGains(const int direction, const size_t channel) const
        {
            SoapySDR_logf(SOAPY_SDR_WARNING, "List gains");
            std::vector<std::string> results;

            // TODO: Add Attenuator Gain
            // TODO: Add Amplifier Gain
            results.push_back("IF");

            return results;
        }

        bool hasGainMode(const int direction, const size_t channel)
        {
            SoapySDR_logf(SOAPY_SDR_WARNING, "Has gain mode");
            return false;
        }

        //SoapySDR::RangeList getGainRange(const int direction, const size_t channel)
        //{
            //SoapySDR::RangeList results;

            //if(direction == SOAPY_SDR_RX) {
                //results.push_back(SoapySDR::Range(0, 31));
            //} else {
                //results.push_back(SoapySDR::Range(0, 89));
            //}

            //return results;
        //}

        //SoapySDR::Range getGainRange(const int direction, const size_t channel, const std::string &name) const
        //{
            //if(direction == SOAPY_SDR_RX) {
                //return SoapySDR::Range(0, 31);
            //} else {
                //return SoapySDR::Range(0, 89);
            //}
        //}

        /*******************************************************************
        * Frequency API
        ******************************************************************/

        void setFrequency(
                const int direction,
                const size_t channel,
                const std::string &name,
                const double frequency,
                const SoapySDR::Kwargs &args)
        {
            SoapySDR_logf(SOAPY_SDR_WARNING, "Set frequency");

            if(direction == SOAPY_SDR_RX) {
                rx_freq = frequency;
            } else {
                tx_freq = frequency;
            }

            if(!running) {
                return;
            }

            if(direction == SOAPY_SDR_RX) {
                sdr_handler->rx_freq(sdr, frequency);
            } else {
                sdr_handler->tx_freq(sdr, frequency);
            }
        }

        SoapySDR::RangeList getFrequencyRange(
                const int direction,
                const size_t channel,
                const std::string &name) const
        {
            SoapySDR_logf(SOAPY_SDR_WARNING, "Range list");

            SoapySDR::RangeList results;

            if(direction == SOAPY_SDR_RX) {
                results.push_back(SoapySDR::Range(rx_freq_min, rx_freq_max));
            } else {
                results.push_back(SoapySDR::Range(tx_freq_min, tx_freq_max));
            }

            return results;
        }

        std::vector<std::string> listFrequencies(const int direction, const size_t channel) const
        {
            SoapySDR_logf(SOAPY_SDR_WARNING, "List frequencies");

            std::vector<std::string> names;
            names.push_back("RF");
            return names;
        }

        // TODO: no idea about this
        bool hasFrequencyCorrection(const int direction, const size_t channel) const
        {
            SoapySDR_logf(SOAPY_SDR_WARNING, "Has frequency correction");

            return false;
        }

        /*******************************************************************
        * Sample Rate API
        ******************************************************************/

        std::vector<double> listSampleRates(const int direction, const size_t channel) const
        {
            SoapySDR_logf(SOAPY_SDR_WARNING, "List sample rates");

            std::vector<double> results;

            results.push_back(250000);
            results.push_back(1024000);
            results.push_back(1536000);
            results.push_back(1792000);
            results.push_back(1920000);
            results.push_back(2048000);
            results.push_back(2160000);
            results.push_back(2560000);
            results.push_back(2880000);
            results.push_back(3200000);

            return results;
        }

        SoapySDR::RangeList getSampleRateRange(const int direction, const size_t channel) const
        {
            SoapySDR_logf(SOAPY_SDR_WARNING, "Get sample rate range");

            SoapySDR::RangeList results;

            if(direction == SOAPY_SDR_RX) {
                results.push_back(SoapySDR::Range(rx_bw_min, rx_bw_max));
            } else {
                results.push_back(SoapySDR::Range(tx_bw_min, tx_bw_max));
            }

            return results;
        }

        void setSampleRate(const int direction, const size_t channel, const double rate)
        {
            SoapySDR_logf(SOAPY_SDR_WARNING, "Set sample rate");

            if(direction == SOAPY_SDR_RX) {
                rx_bw = rate;
            } else {
                tx_bw = rate;
            }

            if(!running) {
                return;
            }

            // Figure out the types
            if(direction == SOAPY_SDR_RX) {
                SoapySDR_logf(SOAPY_SDR_WARNING, "Setting sample rate");
                sdr_handler->rx_bw(sdr, rate);
                SoapySDR_logf(SOAPY_SDR_WARNING, "Set sample rate");
            } else {
                sdr_handler->tx_samp_rate(sdr, rate);
            }
        }

        /*******************************************************************
        * Bandwidth API
        ******************************************************************/
    private:
        SoapySDR::Stream* const TX_STREAM = (SoapySDR::Stream*) 0x1;
        SoapySDR::Stream* const RX_STREAM = (SoapySDR::Stream*) 0x2;
};

/***********************************************************************
 * Find available devices
 **********************************************************************/
SoapySDR::KwargsList findKC908(const SoapySDR::Kwargs &args)
{
    std::vector<SoapySDR::Kwargs> results;

    sdr_api* sdr_handler = kcsdr_init();
    sdr_obj* sdr = sdr_handler->find(KC_908_1);

    if(sdr) {
        SoapySDR::Kwargs devInfo;

        devInfo["name"] = std::string(sdr->name);

        results.push_back(devInfo);
    }

    sdr_handler->close(sdr);

    return results;
}

/***********************************************************************
 * Make device instance
 **********************************************************************/
SoapySDR::Device *makeKC908(const SoapySDR::Kwargs &args)
{
    return new KC908(args);
}

/***********************************************************************
 * Registration
 **********************************************************************/
static SoapySDR::Registry registerKC908("kc908", &findKC908, &makeKC908, SOAPY_SDR_ABI_VERSION);
