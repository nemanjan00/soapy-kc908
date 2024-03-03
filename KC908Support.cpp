#include <SoapySDR/Device.hpp>
#include <SoapySDR/Registry.hpp>
#include"kcsdr.h"

/***********************************************************************
 * Device interface
 **********************************************************************/
class KC908 : public SoapySDR::Device
{
    public:
        sdr_api* sdr_handler;
        sdr_obj* sdr;

        KC908(const SoapySDR::Kwargs &args) {
            (void) args;

            sdr_handler = kcsdr_init();
            sdr = sdr_handler->find(KC_908_1);

            for(int i = 0; i < DEVICE_PORT; i++) {
                printf("\n\n%d\n\n", sdr->port[0].rx_freq.minimum);
            }
        }

        size_t getNumChannels(const int dir) const
        {
            return 1;
        }

        void setFrequency(
                const int direction,
                const size_t channel,
                const std::string &name,
                const double frequency,
                const SoapySDR::Kwargs &args)
        {
            if(direction == SOAPY_SDR_RX) {
                sdr_handler->rx_freq(sdr, frequency);
            }
        }
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
    sdr_api* sdr_handler = kcsdr_init();
    sdr_obj* sdr = sdr_handler->find(KC_908_1);

    return new KC908(args);
}

/***********************************************************************
 * Registration
 **********************************************************************/
static SoapySDR::Registry registerKC908("kc908", &findKC908, &makeKC908, SOAPY_SDR_ABI_VERSION);
