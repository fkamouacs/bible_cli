#include "../include/utils.h"

std::vector<std::string> reorderVector(const std::vector<std::string> &matrix, int rows, int cols)
{
    if (matrix.empty())
    {
        std::cerr << "Error: Input matrix is empty!" << std::endl;
        return {};
    }
    if (rows * cols != matrix.size())
    {
        std::cerr << "Error: Matrix size mismatch!" << std::endl;
        return {};
    }

    std::vector<std::string> reorderd_matrix(cols * rows);

    for (int col = 0; col < cols; col++)
    { // For each column
        for (int row = 0; row < rows; row++)
        { // For each row
            // Calculate where this element should go in the output
            int outputIndex = row * cols + col;

            // Calculate where this element is in the input
            int inputIndex = col * rows + row;

            reorderd_matrix[outputIndex] = matrix[inputIndex];
        }
    }

    return reorderd_matrix;
}

void printMultilineAscii(int start_y, int start_x, const std::string &asciiArt)
{
    std::istringstream stream(asciiArt);
    std::string line;
    int y = start_y;

    while (std::getline(stream, line))
    {
        mvprintw(y++, start_x, "%s", line.c_str()); // Print each line and move to the next row
    }
}