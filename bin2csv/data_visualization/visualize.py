import matplotlib.pyplot as plt
import csv
import numpy as np

t = []
y = []

with open('../F6.csv', 'r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    i = 0
    for row in plots:
        if i > 10000:
            t.append(float(row[0]))
            y.append(float(row[11]))
        if i > 20000:
            break
        i = i + 1

# out = np.fft.fft(y)
# freq = np.fft.fftfreq(len(t))

plt.plot(t, y, label='Loaded from file!')
plt.xlabel('t')
plt.ylabel('y')
plt.title('Interesting Graph\nCheck it out')
plt.legend()
plt.show()
