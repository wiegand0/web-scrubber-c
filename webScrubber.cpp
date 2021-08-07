// REQUIREMENTS:
    // compiled to executable file, runs on windows or any executable compatible platform
    // Runs on the libcurl library, linked, and included in dependencies
// LIMITATIONS:
    // This program does not verify the authenticity/proper formatting for any address it is given, and will simply exit should it be given an invalid address
    // The regular expression does not account for links not addressed as http/https
    // The html is formatted as the curl library provides it and is not in any way cleaned up after fetching
    // Libcurl setup options are diverse, this is one specific interpretation of the problem which follows any redirects to fetch html

#define CURL_STATICLIB
#include <iostream>
#include <stdio.h>
#include <regex>
#include <sstream>
#include "curl/curl.h"


//a struct for storing the memory from the webpage
struct MemStruct {
    char* memory;
    size_t size;
};

//a function to split user input into seperate urls
std::vector<std::string> split_me(std::string me) {
    std::istringstream ss(me);

    //container for seperated urls
    std::vector<std::string> webList;

    //temp string container
    std::string link;

    //put each address into webList vector
    while (ss >> link)
        webList.push_back(link);

    return webList;
}

/*
//a write function so data is not sent to std::cout
static size_t write_data(void* content, size_t size, size_t nmemb, void* userp)
{
    //size of data
    size_t realsize = size * nmemb;
    struct MemStruct* mem = (struct MemStruct*)userp;

    //realloc webPage Struct
    char *ptr = (char *)realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        printf("realloc returned NULL \n");
        return 0;
    }

    //adjust data
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), content, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}*/

//function to handle regex search
void regex_search(std::string html) {

    std::smatch matches;
    //regular expression to search page with
    std::regex regTest("https?://(www.)?[-a-zA-Z0-9@:%._+~#=]{1,256}.[a-zA-Z0-9()]{1,6}\\b([-a-zA-Z0-9()@:%_+.~#?&//=]*)");

    //regex iterators
    auto matches_begin = std::sregex_iterator(html.begin(), html.end(), regTest);
    auto matches_end = std::sregex_iterator();

    //count the matches
    std::cout << std::distance(matches_begin, matches_end) << "\n";

}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

//function to handle all of the curl based code
void curl_me(std::string webSite) {

    //initialize curl variables
    CURL* curl;
    CURLcode result;

    //store the webpage
    std::string readBuffer;

    //initialize curl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        //point to url to scrub
        curl_easy_setopt(curl, CURLOPT_URL, webSite.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        //send data to struct
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        //store result
        result = curl_easy_perform(curl);
        //if the curl fails print the error
        if (result != CURLE_OK) {
            fprintf(stderr, curl_easy_strerror(result), "\n");
            std::cout << "\n";
        }
        //otherwise continue with the scrub
        else {
            std::cout << webSite << " ";
            regex_search(readBuffer);
        }

        //cleanup
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}

int main(int argc, char* argv[])
{
    std::string address;

    std::vector<std::string> fixedList;

    //if there are no arguments given, ask for some, place arguments into vector
    if (argc == 1) {
        std::cout << "URLs to be scrubbed: ";
        std::getline(std::cin, address);
        fixedList = split_me(address);
    } else 
        for (int i = 1; i < argc; i++) 
            fixedList.push_back(argv[i]);
 


    for (auto it = fixedList.begin(); it != fixedList.end(); it++) {
        curl_me(*it);
    }

    system("pause");
}