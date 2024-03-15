import SoapySDR
from SoapySDR import * #SOAPY_SDR_* constants
import numpy as np
import time

if __name__ == "__main__":
    kc908 = SoapySDR.Device(dict(driver="kc908"))

    kc908.setSampleRate(SOAPY_SDR_RX, 0, 1e6)
    kc908.setSampleRate(SOAPY_SDR_TX, 0, 1e6)

    """
    for i in range(5):
        print("  Make rx stream #%d"%i)
        rxStream = kc908.setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32, [0])
        for j in range(5):
            numSampsTotal = 10000
            print("    Activate, get %d samples, Deactivate #%d"%(numSampsTotal, j))
            kc908.activateStream(rxStream)
            buff = np.array([0]*1024, np.complex64)
            while numSampsTotal > 0:
                sr = kc908.readStream(rxStream, [buff], buff.size, timeoutUs=int(1e6))
                #print sr
                assert(sr.ret > 0)
                numSampsTotal -= sr.ret
            kc908.deactivateStream(rxStream)
        kc908.closeStream(rxStream)

    for i in range(5):
        print("  Make tx stream #%d"%i)
        txStream = kc908.setupStream(SOAPY_SDR_TX, SOAPY_SDR_CF32, [0])
        for j in range(5):
            numSampsTotal = 10000
            print("    Activate, send %d samples, Deactivate #%d"%(numSampsTotal, j))
            kc908.activateStream(txStream)
            buff = np.array([0]*1024, np.complex64)
            while numSampsTotal != 0:
                size = min(buff.size, numSampsTotal)
                sr = kc908.writeStream(txStream, [buff], size)
                #print sr
                if not (sr.ret > 0): print("Fail %s, %d"%(sr, numSampsTotal))
                assert(sr.ret > 0)
                numSampsTotal -= sr.ret
            kc908.deactivateStream(txStream)
        kc908.closeStream(txStream)
    """

    ####################################################################
    #setup both streams at once
    ####################################################################
    rxStream = kc908.setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32, [0])
    txStream = kc908.setupStream(SOAPY_SDR_TX, SOAPY_SDR_CF32, [0])

    kc908.activateStream(rxStream)
    kc908.activateStream(txStream)

    numSampsTotal = 10000
    kc908.activateStream(rxStream)
    buff = np.array([0]*1024, np.complex64)
    while numSampsTotal > 0:
        sr = kc908.readStream(rxStream, [buff], buff.size, timeoutUs=int(1e6))
        #print sr
        assert(sr.ret > 0)
        numSampsTotal -= sr.ret

    numSampsTotal = 10000
    buff = np.array([0]*1024, np.complex64)
    while numSampsTotal != 0:
        size = min(buff.size, numSampsTotal)
        sr = kc908.writeStream(txStream, [buff], size)
        #print sr
        if not (sr.ret > 0): print("Fail %s, %d"%(sr, numSampsTotal))
        assert(sr.ret > 0)
        numSampsTotal -= sr.ret

    kc908.deactivateStream(rxStream)
    kc908.deactivateStream(txStream)

    kc908.closeStream(rxStream)
    kc908.closeStream(txStream)
