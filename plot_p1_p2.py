import matplotlib.pyplot as plt
import csv
from decimal import Decimal

x = []
y = []

with open('time1800.csv','r') as csvfile:
	plots = csv.reader(csvfile, delimiter = ',')
	
	for row in plots:
		x.append(int(row[0]))
		y.append(Decimal(row[1]))

plt.plot(x, y, color = 'g')
plt.legend()
plt.xlabel('threads')
plt.ylabel('time')
plt.title('P1 for 800*999 999*382 matrices ')
plt.show()

