#include "image_manipulation.h"


// There is some precision loss with the GMP library. Getting a max of +- 200 difference on the final hash on each
// execution. This is caused by the nature of GMP, which stores big integers like floats. It should not be an issue
// considering the absurd high value of the final hash.
mpz_class Image::hash10x10()
{
    Mat matrix10px;

    cv::resize( matrix, matrix10px, {10, 10} );
    cvtColor( matrix10px, matrix10px, cv::COLOR_BGR2GRAY );

    mpz_class differenceHash{0};

    uchar pixel, previousPixel = matrix10px . at<uchar>( 9, 0 );

    for ( int row = 0; row < 10; row += 2 )
    {
        for ( int col = 0; col < 10; col++ )
        {
            differenceHash <<= 1;
            pixel = matrix10px . at<uchar>( row, col );
            differenceHash |= 1 * ( pixel >= previousPixel );
            previousPixel = pixel;
        }
        row++;

        for ( int col = 9; col > -1; col-- )
        {
            differenceHash <<= 1;
            pixel = matrix10px . at<uchar>( row, col );

            differenceHash |= 1 * ( pixel >= previousPixel );

            previousPixel = pixel;
        }
    }
    return differenceHash;
}

// No precision loss on this method considering the much smaller hash.
mpz_class Image::hash8x8()
{
    Mat matrix8px;

    cv::resize( matrix, matrix8px, {8, 8} );
    cvtColor( matrix8px, matrix8px, cv::COLOR_BGR2GRAY );


    mpz_class differenceHash{0};

    uchar pixel, previousPixel = matrix8px . at<uchar>( 7, 0 );

    for ( int row = 0; row < 8; row += 2 )
    {
        for ( int col = 0; col < 8; col++ )
        {
            differenceHash <<= 1;
            pixel = matrix8px . at<uchar>( row, col );
            differenceHash |= 1 * ( pixel >= previousPixel );
            previousPixel = pixel;
        }
        row++;

        for ( int col = 7; col > -1; col-- )
        {
            differenceHash <<= 1;
            pixel = matrix8px . at<uchar>( row, col );

            differenceHash |= 1 * ( pixel >= previousPixel );

            previousPixel = pixel;
        }
    }
    return differenceHash;
}
