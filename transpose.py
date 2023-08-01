import numpy as np

import numpy as np
matrix = np.loadtxt("in2.txt", dtype='int', delimiter=' ')

res = np.zeros((len(matrix[0]),len(matrix)),dtype='int')

for i in range(len(matrix)):
   for j in range(len(matrix[0])):
       res[j][i] = matrix[i][j]

np.savetxt("in2.txt", res, fmt='%d')
