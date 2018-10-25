import numpy as np

rows = 28
cols = 10

H3 = np.random.choice(a=[0,1], size=(rows,cols));

#print col oriented
print "{",
for i in xrange(rows):
	for j in xrange(cols):
		print H3[i,j],",",
print "}"
