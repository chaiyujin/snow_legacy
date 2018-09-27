import math
import numpy as np


FACTOR = -0.5
DIS_SIG = 1
RAG_SIG = 1
RADIUS = 5


class BilateralFilter1D(object):
    '''
        distance_sigma: distance sigma,
        rs: range sigma
        radius: half length of gaussian kernel
    '''
    def __init__(self, factor=FACTOR, distance_sigma=DIS_SIG,
                 range_sigma=RAG_SIG, radius=RADIUS):
        self._factor = float(factor)
        self._ds = float(distance_sigma)
        self._rs = float(range_sigma)
        self._radius = int(radius)
        self.__build_distance_weight_table()
        self.__build_similarity_weight_table()

    def __build_distance_weight_table(self):
        self._distance_w = []
        for idx in range(-self._radius, self._radius + 1):
            delta = (float(idx) / self._ds)
            self._distance_w.append(
                math.exp(delta * delta * self._factor))

    def __build_similarity_weight_table(self):
        pass

    def distance_weight(self, dis):
        return self._distance_w[dis + self._radius]

    def similarity_weight(self, delta):
        delta = math.sqrt(delta * delta) / self._rs
        return math.exp(delta * delta * self._factor)

    def filter(self, signal):
        signal = np.asarray(signal)
        shape = signal.shape
        # reshape the signal
        if len(signal.shape) > 2:
            w = 1
            for i in range(1, len(signal.shape)):
                w *= signal.shape[i]
            signal = np.reshape(signal, (len(signal), w))
        # expand if needed
        if len(signal.shape) == 1:
            signal = np.reshape(signal, (len(signal), 1))
        # filter
        r = self._radius
        new_sig = []
        for idx in range(len(signal)):
            tmp = []
            for dim in range(len(signal[idx])):
                mean = 0.0
                ws = 0.0
                for di in range(-r, r + 1):
                    ni = di + idx
                    if ni < 0 or ni >= len(signal):
                        continue
                    dw = self.distance_weight(di)
                    sw = self.similarity_weight(
                        signal[idx][dim] - signal[ni][dim])
                    weight = dw * sw
                    ws += weight
                    mean += weight * signal[ni][dim]
                mean /= ws
                tmp.append(mean)
            new_sig.append(tmp)
        return np.reshape(new_sig, shape)


if __name__ == "__main__":
    import numpy as np
    signal = [0, 2.1, 1.7, 2.2, 1.9, 2.1, 5, 10, 5, 2.2, 1.9, 1.95, 2.04, 1.99, 2.10, 1.98]
    filter = BilateralFilter1D()
    new = filter.filter(signal)
    for i in range(len(signal)):
        print("%.4f" % signal[i], end='\t')
    print()
    for i in range(len(new)):
        print("%.4f" % new[i], end='\t')
    # signal = [[0, 0], [1, 1]]
    # filter = BilateralFilter1D(-0.5, 1, 1, 5)
    # new = filter.filter(signal)
    # print(new)
