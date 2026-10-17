//
// Created by Logan Drescher on 3/18/25.
//

#include <fstream>
#include <filesystem>
#include <gtest/gtest.h>
#include "ComplexDataStructures/Hashtable.h"
#include "../build/VCell/tests/testFiles/input/testResourceLocations.h" // provides testResourceLocations

class CHashtableTest : public testing::Test {
    public:
        void SetUp() override {
            std::filesystem::path inputFile = testResourceLocations::EXAMPLE_SAMPLE_HASH_TEST_INPUT;
            ASSERT_TRUE(std::filesystem::exists(inputFile)) << "Relevant input doesn't exist; have path names changed?\n";
            this->inputStream.open(inputFile.string());
            ASSERT_TRUE(this->inputStream.is_open()) << "Unable to open file\n";

            std::string line{};
            while (std::getline(inputStream, line)) {
                if (counts->contains(counts, line.c_str())) {
                    void* value = counts->getFrom(counts, line.c_str());
                    // Already exists, increment int that value points to.
                    int* pcount = (int*)value;
                    (*pcount)++;
                    continue;
                }
                // Word not found, allocate space for new int and set to 1.
                int* pcount = (int*)malloc(sizeof(int));
                ASSERT_NE(pcount, nullptr) << "Memory allocation of count-value failed!\n";
                *pcount = 1;
                ASSERT_TRUE(counts->putIn(counts, line.c_str(), pcount)) << "Unable to put new counts in hashtable!\n";
            }
        }

        void TearDown() override {
            this->inputStream.close();
        }

    protected:
        CHashtableTest() {
            this->counts = createNewHashtable();
        }

        ~CHashtableTest() override {
            this->counts->freeThis(this->counts);
            this->counts = nullptr;
        }

        Hashtable* counts;
        std::ifstream inputStream{};
};

TEST_F(CHashtableTest, SpotTest) {
    if (!this->counts->contains(this->counts, "pizzas")) {fprintf(stderr, "Hashtable does not contain `pizzas`!\n"); FAIL();}
    if (2 != *(int*)this->counts->getFrom(this->counts, "pizzas")) {fprintf(stderr, "Hashtable contains incorrect number of `pizzas`!\n"); FAIL();}
    if (this->counts->contains(this->counts, "hackathons")) {fprintf(stderr, "Hashtable contains `hackathons`, when it shouldn't!\n"); FAIL();}
    errno = 0;
    if (this->counts->getFrom(this->counts, "hackathons") != NULL || errno == 0) {fprintf(stderr, "Bad Access did not properly error out!\n"); FAIL();}
}

TEST_F(CHashtableTest, IteratorTest) {
    HashtableIterator it = this->counts->getIterator(this->counts);
    int counter;
    for (counter = 0; it.iter(&it); counter++) {
        EXPECT_TRUE(this->counts->contains(this->counts, it.key)) << "Iterator does not produce valid keys!\n";
        free(it.value);
    }
    EXPECT_EQ(counter, this->counts->size(this->counts)) << "Iterator size does not match Hashtable size!\n";
}
