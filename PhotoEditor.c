#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

typedef uint8_t BYTE;
typedef uint32_t DWORD;
typedef int32_t LONG;
typedef uint16_t WORD;

typedef struct
{
    WORD bfType;  //if not 0x4d42, then not a bitmap file
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} __attribute__((__packed__))
BITMAPFILEHEADER;

typedef struct
{
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} __attribute__((__packed__))
BITMAPINFOHEADER;

typedef struct
{
    BYTE rgbtBlue;
    BYTE rgbtGreen;
    BYTE rgbtRed;
} __attribute__((__packed__))
RGBTRIPLE;

void grayscale(int height, int width, RGBTRIPLE image[height][width])
{
    int avg;

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            avg = (int)((image[i][j].rgbtBlue + image[i][j].rgbtGreen + image[i][j].rgbtRed) / 3.0 + 0.5);

            image[i][j].rgbtRed = avg;
            image[i][j].rgbtGreen = avg;
            image[i][j].rgbtBlue = avg;
        }
    }
}

void negative(int height, int width, RGBTRIPLE image[height][width])
{

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            image[i][j].rgbtBlue = 255 - image[i][j].rgbtBlue;
            image[i][j].rgbtRed = 255 - image[i][j].rgbtRed;
            image[i][j].rgbtGreen = 255 - image[i][j].rgbtGreen;
        }
    }


}

void sepia(int height, int width, RGBTRIPLE image[height][width])
{
    float sepiaRed, sepiaBlue, sepiaGreen;

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            sepiaRed = fmin(255, (0.393 * image[i][j].rgbtRed) + (0.769 * image[i][j].rgbtGreen) + (0.189 * image[i][j].rgbtBlue) + 0.5);
            sepiaBlue = fmin(255, (0.272 * image[i][j].rgbtRed) + (0.534 * image[i][j].rgbtGreen) + (0.131 * image[i][j].rgbtBlue) + 0.5);
            sepiaGreen = fmin(255, (0.349 * image[i][j].rgbtRed) + (0.686 * image[i][j].rgbtGreen) + (0.168 * image[i][j].rgbtBlue) + 0.5);

            image[i][j].rgbtBlue = (int)sepiaBlue;
            image[i][j].rgbtRed = (int)sepiaRed;
            image[i][j].rgbtGreen = (int)sepiaGreen;
        }
    }
}

void reflect(int height, int width, RGBTRIPLE image[height][width])
{
    int half = (int)(width / 2);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < half; j++)
        {
            RGBTRIPLE temp = image[i][j];
            image[i][j] = image[i][width - 1 - j];
            image[i][width - 1 - j] = temp;
        }
    }
}

void blur(int height, int width, RGBTRIPLE image[height][width])
{
    RGBTRIPLE temp[height][width];

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            temp[i][j] = image[i][j];
        }
    }

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int totalRed = 0, totalGreen = 0, totalBlue = 0;
            float counter = 0.00;

            for (int x = -2; x < 3; x++)
            {
                for (int y = -2; y < 3; y++)
                {
                    int currentX = i + x;
                    int currentY = j + y;

                    if (currentX >= 0 && currentX < height && currentY >= 0 && currentY < width)
                    {
                        totalRed += image[currentX][currentY].rgbtRed;
                        totalGreen += image[currentX][currentY].rgbtGreen;
                        totalBlue += image[currentX][currentY].rgbtBlue;
                        counter++;
                    }
                }
            }

            temp[i][j].rgbtRed = round(totalRed / counter);
            temp[i][j].rgbtGreen = round(totalGreen / counter);
            temp[i][j].rgbtBlue = round(totalBlue / counter);
        }
    }

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            image[i][j] = temp[i][j];
        }
    }
}

int main()
{
    // Define allowable filters
    char *filters = "bgrsn";

    char filter;
    printf("========== Filter Menu ==========\n");
    printf("b: Blur\n");
    printf("s: Sepia\n");
    printf("r: Reflect\n");
    printf("g: Greyscale\n");
    printf("n: Negative\n");
    printf("=================================\n");

    // Prompt for filter selection
    printf("Enter the filter you want to apply: ");
    scanf(" %c", &filter);

    // Open input file
    char infile[50];
    char outfile[50];
    printf("Enter input file name: ");
    scanf("%s", infile);

    printf("Enter output file name: ");
    scanf("%s", outfile);

    FILE *inptr = fopen(infile, "rb");
    if (inptr == NULL)
    {
        printf("Could not open %s.\n", infile);
        return 1;
    }

    // Read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // Read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // Ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(inptr);
        printf("Unsupported file format.\n");
        return 2;
    }

    // Get image's dimensions
    int height = abs(bi.biHeight);
    int width = bi.biWidth;

    // Allocate memory for image
    RGBTRIPLE(*image)[width] = calloc(height, sizeof(RGBTRIPLE[width]));
    if (image == NULL)
    {
        printf("Not enough memory to store image.\n");
        fclose(inptr);
        return 3;
    }

    // Determine padding for scanlines
    int padding = (4 - (width * sizeof(RGBTRIPLE)) % 4) % 4;

    // Iterate over infile's scanlines
    for (int i = 0; i < height; i++)
    {
        // Read row into pixel array
        fread(image[i], sizeof(RGBTRIPLE), width, inptr);

        // Skip over padding
        fseek(inptr, padding, SEEK_CUR);
    }

    // Filter image
    switch (filter)
    {
    // Blur
    case 'b':
        blur(height, width, image);
        break;

    // Grayscale
    case 'g':
        grayscale(height, width, image);
        break;

    // Reflection
    case 'r':
        reflect(height, width, image);
        break;

    // Sepia
    case 's':
        sepia(height, width, image);
        break;
    // Negative
    case 'n':
        negative(height, width, image);
        break;
    }

    // Open output file
    FILE *outptr = fopen(outfile, "wb");
    if (outptr == NULL)
    {
        fclose(inptr);
        free(image);
        printf("Could not create %s.\n", outfile);
        return 4;
    }

    // Write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // Write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    // Write new pixels to outfile
    for (int i = 0; i < height; i++)
    {
        // Write row to outfile
        fwrite(image[i], sizeof(RGBTRIPLE), width, outptr);

        // Write padding at the end of the row
        for (int k = 0; k < padding; k++)
        {
            fputc(0x00, outptr);
        }
    }

    // Close files
    fclose(inptr);
    fclose(outptr);

    // Free memory for image
    free(image);

    return 0;
}
