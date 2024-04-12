import matplotlib.pyplot as plt
import sys

fig, ax = plt.subplots()

ys = [int(n) for n in sys.argv[1:]]
xs = range(len(ys))
ax.plot(xs, ys)

fig.show()