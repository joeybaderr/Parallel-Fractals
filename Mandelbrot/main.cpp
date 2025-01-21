#include "mpi.h"
#include <complex>
#include <iostream>
#include <fstream>
using namespace std;
using namespace complex_literals;

const int max_iter = 256;
const int width = 1200;
const int height = 1200;
const double rmin = -2.5;
const double rmax = 1.5;
const double imin = -2.0;
const double imax = 2.0;

int getMandelbrotN(complex<double> c)
{

    int n = 0;
    double magnitude = 0;
    complex<double> z(0.0, 0.0);
    while ((n < max_iter) && (magnitude < 4))
    {
        z = z * z + c;
        magnitude = (z.real() * z.real()) + (z.imag() * z.imag());
        n++;
    }
    return n;
}

complex<double> mapPixelToComplexPlane(int x, int y)
{
    // range/width gives step size for each pixel
    double horizontalRange = rmax - rmin;
    double newCReal = x * (horizontalRange / width) + rmin;

    double verticalRange = imax - imin;
    double newCImag = y * (verticalRange / height) + imin;

    return complex<double>(newCReal, newCImag);
}

void sequential()
{

    // open the output file, write the ppm header
    ofstream fout("output_image.ppm");
    fout << "P3" << endl;                   // magic number
    fout << width << " " << height << endl; // our dimensions
    fout << "255" << endl;                  // max value of a rgb pixel

    for (int y = 0; y < height; y++)
    { // row
        for (int x = 0; x < width; x++)
        { // the pixels in each row
            complex<double> c = mapPixelToComplexPlane(x, y);
            int n = getMandelbrotN(c);

            // coloring the pixel
            int color = (n % 256);
            fout << color << " " << color << " " << color << " "; // represents one pixel
        }
        fout << endl;
    }
    fout.close();
    cout << "Finished" << endl;
}

void parallelStatic(const int size, const int rank)
{
    int rowIncrement = height / (size - 1); // size - 1 because master node will not be processing
    // The last rank (rank == size - 1) adjusts its endpoint to ensure all remaining rows up to 'height' are processed.

    if (rank == 0)
    {
        char processor[MPI_MAX_PROCESSOR_NAME];
        int processorNameLength;
        MPI_Get_processor_name(processor, &processorNameLength);
        cout << processor << " rank " << rank << ": Sending rows..." << endl;
        MPI_Barrier(MPI_COMM_WORLD);
        int row = 0;
        for (int i = 1; i < size; i++)
        {
            MPI_Send(&row, 1, MPI_INT, i, 0, MPI_COMM_WORLD); // tag=0
            row += rowIncrement;
        }
        cout << processor << " rank " << rank << ": Rows sent." << endl;

        cout << processor << " rank " << rank << ": Recieving rows... (LISTENING)" << endl;

        MPI_Barrier(MPI_COMM_WORLD);

        int **rows = new int *[height];
        for (int i = 0; i < height; ++i)
        {
            rows[i] = new int[width];
        }
        int pixel[3];

        for (int i = 0; i < (width * height); i++)
        {
            MPI_Recv(pixel, 3, MPI_INT, MPI_ANY_SOURCE, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            rows[pixel[2]][pixel[1]] = pixel[0];
        }
        cout << processor << " rank " << rank << ": Rows recieved." << endl;

        MPI_Barrier(MPI_COMM_WORLD);

        cout << processor << " rank " << rank << ": Coloring image..." << endl;
        ofstream fout("parallel_output_image.ppm");
        fout << "P3" << endl;                   // magic number
        fout << width << " " << height << endl; // our dimensions
        fout << "255" << endl;                  // max value of a rgb pixel
        int color = 0;
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                color = rows[y][x];
                fout << color << " " << color << " " << color << " "; // represents one pixel
            }
            fout << endl;
        }
        for (int i = 0; i < height; ++i)
        {
            delete[] rows[i];
        }
        delete[] rows;
        fout.close();
        cout << processor << " rank " << rank << ": Image has been colored." << endl;
    }
    else
    { // else if slave node -> process and return a color (int)
        char processor[MPI_MAX_PROCESSOR_NAME];
        int processorNameLength;
        MPI_Get_processor_name(processor, &processorNameLength);
        MPI_Barrier(MPI_COMM_WORLD);
        int rows = 0;
        cout << processor << " rank " << rank << ": Recieving rows..." << endl;
        MPI_Recv(&rows, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        cout << processor << " rank " << rank << ": Recieved rows, " << " starting at " << rows << endl;

        MPI_Barrier(MPI_COMM_WORLD);

        cout << processor << " rank " << rank << ": Processing colors and gridpoints" << endl;
        int pixel[3];
        int count = 0;
        for (int y = rows; y < (rank == size - 1 ? height : rows + rowIncrement); y++)
        {
            for (int x = 0; x < width; x++)
            {
                complex<double> c = mapPixelToComplexPlane(x, y);
                int n = getMandelbrotN(c); // returns an N
                pixel[0] = n;
                pixel[1] = x;
                pixel[2] = y;
                MPI_Send(pixel, 3, MPI_INT, 0, 3, MPI_COMM_WORLD);
            }
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
}

int main(int argc, char *argv[])
{
        MPI_Init(&argc, &argv);
        int worldSize;
        int worldRank;
        MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
        MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
        parallelStatic(worldSize, worldRank);
        MPI_Finalize();
    
}
