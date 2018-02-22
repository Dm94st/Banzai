# Test-task for Banzai-Games

The program implements 3 methods of interpolation(Nearest Neighbor, Linear, Quadratic). 

Input:
1) File containing the points for which the interpolation is performed.

Format:

t1, f(t1)

t2, f(t2)

...

tk, f(tk)

2) File contains a set of arguments for which it is necessary to find the interpolated values.
Format:

t1

t2

...

tk

Output:

File with the coordinates of the interpolated points.
Format:

t1, f(t1)

t2, f(t2)

...

tk, f(tk)

Using: 
Banzai_Games <Input path until file with source X and Y values(file 1)> <Delimiter> <Input path file with args(file 2)> <Path until output File>;

Example:
Banzai_Games SrcFile1.csv , SrcFile2.csv outFile.csv; 

After starting the program, you can choose Interpolate method:
1) Nearest Neighbor
2) Linear;
3) Quadratic;

SrcFile1.csv - example of file with source X and Y values(file 1);

SrcFile2.csv - example of file with args(file 2);

outFile.csv - example of output file(Quadratic method).
