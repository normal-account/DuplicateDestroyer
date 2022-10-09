#include <chrono>
#include <cstdio>
#include "submission_processing.h"
#include "message_processing.h"

std::vector<std::thread*> benchmarkThreads;
std::binary_semaphore setSemaphore{1};
ApiWrapper apiWrapper(NUMBER_THREADS);
db_interface *interfaces[NUMBER_THREADS + 1];
tesseract::TessBaseAPI* tessBaseApi[NUMBER_THREADS];

using namespace ::mysqlx;

unsigned long long get_unix_time() {
    const auto currentTime = std::chrono::system_clock::now();
    const auto currentTimeUnix = std::chrono::duration_cast<std::chrono::seconds>(currentTime.time_since_epoch()).count();
    return currentTimeUnix;
}

// Should be done only once the original token expires.
void initialize_token()
{
    cpr::Response tokenQuery = apiWrapper.fetch_token();
    json jsonToken = json::parse( tokenQuery . text );
    std::string ogToken = "bearer ";
    ogToken . append( jsonToken[ "access_token" ] );
    apiWrapper.set_token( ogToken );
    int expiresIn = jsonToken["expires_in"];
    apiWrapper.set_time_expire(get_unix_time() + expiresIn);
}

void destroy_instances() {
    for (auto &i : tessBaseApi)
    {
        i -> End();
        delete i;
    }

    for (auto &i : interfaces) {
        delete i;
    }
    std::cout << "[DELETED INSTANCES]" << std::endl;
}

void initialize_tesseract()
{
    for (auto &i : tessBaseApi) {
        i = new tesseract::TessBaseAPI();
        // Initialize tesseract-ocr with English, without specifying tessdata path
        if (i->Init( nullptr, "eng")) {
            fprintf(stderr, "Could not initialize tesseract.\n");
            exit(1);
        }

        // Or PSM_AUTO_OSD for extra info (OSD). Difference not significant.
        // See : https://pyimagesearch.com/2021/11/15/tesseract-page-segmentation-modes-psms-explained-how-to-improve-your-ocr-accuracy/
        i->SetVariable("debug_file", "/dev/null");
        i->SetPageSegMode( tesseract::PSM_AUTO);   //matches -psm 1 from the command line
    }
}

void initialize_db_instances() {
    for (auto &interface : interfaces) {
        interface = new db_interface(Session("localhost", 33060, "db_user", "soleil"));
    }
}

// Adjust sleep in function of new submissions. Don't want to waste our API requests when there are few submissions.
unsigned calculate_sleep() {
    auto substract = (unsigned)( interfaces[NUMBER_THREADS]->get_new_submissions() * (0.3));
    if (substract > 30)
        substract = 30;
    unsigned sleep = 30 - substract;
    return sleep;
}

int main()
{
    assert(NUMBER_THREADS >= 1);
    int count = 0;
    initialize_tesseract();
    initialize_db_instances();
    atexit(( destroy_instances ));

    while (true) {
        try {
            if (apiWrapper.get_time_expire() - get_unix_time() < 10000 || apiWrapper.get_time_expire() == 0)
                initialize_token();

            iterate_messages();
            iterate_submissions();
        } catch (std::exception &e) {
            std::cerr << "EXCEPTION : " << e.what() << std::endl;
        }
        std::cout << count++ << std::endl;
        sleep(/*calculate_sleep()*/10);
    }
}
