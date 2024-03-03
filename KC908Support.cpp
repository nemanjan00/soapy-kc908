#include <SoapySDR/Device.hpp>
#include <SoapySDR/Registry.hpp>
#include <SoapySDR/Formats.hpp>
#include <inttypes.h>
#include"kcsdr.h"

/***********************************************************************
 * Device interface
 **********************************************************************/
class KC908 : public SoapySDR::Device
{
    public:
        sdr_api* sdr_handler;
        sdr_obj* sdr;

        KC908(const SoapySDR::Kwargs &args)
        {
            (void) args;

            sdr_handler = kcsdr_init();
            sdr = sdr_handler->find(KC_908_1);
        }

        /*******************************************************************
        * Channels API
        ******************************************************************/

        size_t getNumChannels(const int dir) const
        {
            return 1;
        }

        bool getFullDuplex(const int direction, const size_t channel) const
        {
            return false;
        }

        /*******************************************************************
        * Stream API
        ******************************************************************/

        std::vector<std::string> getStreamFormats(const int direction, const size_t channel) const {
            std::vector<std::string> formats;

            formats.push_back(SOAPY_SDR_CU16);

            return formats;
        }

        std::string getNativeStreamFormat(const int direction, const size_t channel, double &fullScale) const
        {
            return SOAPY_SDR_CU16;
        }

        /*******************************************************************
        * Gain API
        ******************************************************************/

        std::vector<std::string> listGains(const int direction, const size_t channel) const
        {
            std::vector<std::string> results;

            // TODO: Add Attenuator Gain
            // TODO: Add Amplifier Gain
            results.push_back("IF");

            return results;
        }

        bool hasGainMode(const int direction, const size_t channel)
        {
            return true;
        }

        SoapySDR::RangeList getGainRange(const int direction, const size_t channel)
        {
            SoapySDR::RangeList results;

            if(direction == SOAPY_SDR_RX) {
                results.push_back(SoapySDR::Range(0, 31));
            } else {
                results.push_back(SoapySDR::Range(0, 89));
            }

            return results;
        }

        SoapySDR::Range getGainRange(const int direction, const size_t channel, const std::string &name) const
        {
            if(direction == SOAPY_SDR_RX) {
                return SoapySDR::Range(0, 31);
            } else {
                return SoapySDR::Range(0, 89);
            }
        }

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
            SoapySDR::RangeList results;

            if(direction == SOAPY_SDR_RX) {
                results.push_back(SoapySDR::Range(sdr->port[0].rx_freq.minimum, sdr->port[0].rx_freq.maximum));
            } else {
                results.push_back(SoapySDR::Range(sdr->port[1].tx_freq.minimum, sdr->port[1].tx_freq.maximum));
            }

            return results;
        }

        std::vector<std::string> listFrequencies(const int direction, const size_t channel) const
        {
            std::vector<std::string> names;
            names.push_back("RF");
            return names;
        }

        // TODO: no idea about this
        bool hasFrequencyCorrection(const int direction, const size_t channel) const
        {
            return false;
        }

        /*******************************************************************
        * Sample Rate API
        ******************************************************************/

        SoapySDR::RangeList getSampleRateRange(const int direction, const size_t channel) const
        {

            SoapySDR::RangeList results;

            if(direction == SOAPY_SDR_RX) {
                results.push_back(SoapySDR::Range(sdr->port[0].samp_rate.minimum, sdr->port[0].samp_rate.maximum));
            } else {
                results.push_back(SoapySDR::Range(sdr->port[1].samp_rate.minimum, sdr->port[1].samp_rate.maximum));
            }

            return results;
        }

        void setSampleRate(const int direction, const size_t channel, const double rate)
        {
            // Figure out the types
            if(direction == SOAPY_SDR_RX) {
                sdr_handler->rx_samp_rate(sdr, rate);
            } else {
                sdr_handler->tx_samp_rate(sdr, rate);
            }
        }

        /*******************************************************************
        * Bandwidth API
        ******************************************************************/
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
