#include "image_manipulation.h"

extern tesseract::TessBaseAPI* tessBaseApi[NUMBER_THREADS];

void Image::computeHash10x10()
{
    Mat matrix10px;

    //cv::
    cv::resize( matrix, matrix10px, {11, 11} ); // Previously 10x10
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
    this->hash10x10 = differenceHash;
}

void Image::computeHash8x8()
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
    this->hash8x8 = differenceHash;
}

int Image::compareHash10x10( const mpz_class &hash) {
    return compareHash(this->hash10x10, hash);
}

int Image::compareHash8x8( const mpz_class &hash) {
    return compareHash(this->hash8x8, hash);
}

int Image::compareHash( const mpz_class &hash1, const mpz_class &hash2)
{
    mpz_class xorHash = hash1 ^ hash2;
    std::string binaryHash = xorHash.get_str(2);
    int count = 0;
    for (char i : binaryHash) {
        if (i == '1')
            count++;
    }
    int mediaSimilarity = int(( (64 - count ) * 100) / 64.0);
    return mediaSimilarity;
}

void Image::extract_text(int threadNumber)
{
    // TODO : Bug in Tess that causes rare infinite loops on GetUTF8. Either add a timeout or figure out a better fix...
    auto api = tessBaseApi[threadNumber];
    try {
        Pix *pix = pixRead(determine_image_name(threadNumber).c_str());
        api->SetImage( pix );
        ocrText = api->GetUTF8Text();
        if (ocrText.size() > 300)
            ocrText = ocrText.substr(0, 300);
    }
    catch (std::exception &e) {
        std::cerr << "EXCEPTION ON TEXT EXTRACTION : " << e.what() << std::endl;
        ocrText = "";
    }
}

// To consider a modified version of the Levenshtein algorithm that takes string length in consideration
int getEditDistance(const std::string &first, const std::string& second)
{
    int m = first.length();
    int n = second.length();

    int T[m + 1][n + 1];
    T[0][0] = 0;
    for (int i = 1; i <= m; i++) {
        T[i][0] = i;
    }

    for (int j = 1; j <= n; j++) {
        T[0][j] = j;
    }

    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            int weight = first[i - 1] == second[j - 1] ? 0: 1;
            T[i][j] = std::min(std::min(T[i-1][j] + 1, T[i][j-1] + 1), T[i-1][j-1] + weight);
        }
    }

    return T[m][n];
}

double get_string_similarity(const std::string &first, const std::string &second) {
    double max_length = std::max(first.length(), second.length());
    if (max_length > 0) {
        return ((max_length - getEditDistance(first, second)) / max_length ) * 100;
    }
    return 100.0;
}

std::string Image::determine_image_name(int threadNumber) {
    return "image_in_process" + std::to_string(threadNumber);
}

std::string Image::prepare_word( std::string &word )
{
    std::string newWord;
    newWord.reserve(word.size());
    for (char i : word) {
        if (i >= 'a') //
            newWord.push_back(i);
        else if (i >= 'A' && i <= 'Z')
            newWord.push_back(i + 0x20);
    }
    return newWord;
}

std::string Image::filter_non_words( const std::set<std::string> &dict ) const
{
    std::string filteredOcr;
    filteredOcr.reserve(ocrText.size());
    std::stringstream ocrStream(ocrText);
    std::string word;
    do {
        ocrStream >> word;
        std::string preparedWord = prepare_word(word);
        if (dict.contains(preparedWord))
            filteredOcr.append(preparedWord).append(" ");
    } while(!ocrStream.eof() && ocrStream.tellg() != -1);
    std::cout << "Before {" << ocrText << "}" << std::endl;
    std::cout << "After  {" << filteredOcr << "}" << std::endl;
    return filteredOcr;
}

cv::Size Image::get_dimensions() const {
    return matrix.size();
}

bool Image::isValidImage( const std::string &imageName )
{
    std::ifstream file(imageName);
    std::string temp;
    temp.resize(6);
    char* begin = &*temp.begin();
    file.read(begin, 5);
    auto* h = reinterpret_cast<unsigned char *>(begin);

    //GIF8
    if (h[0] == 71 && h[1] == 73 && h[2] == 70 && h[3] == 56)
        return true;

    //89 PNG
    if (h[0] == 137 && h[1] == 80 && h[2] == 78 && h[3] == 71)
        return true;

    //FFD8 PNG
    if (h[0] == 255 && h[1] == 216)
        return true;

    return false;
}