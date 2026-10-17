//
// Created by Logan Drescher on 3/19/25.
//

//
// Created by Logan Drescher on 3/19/25.
//

#include <gtest/gtest.h>
#include "ComplexDataStructures/Vector.h"

class CVectorTest : public testing::Test {
    public:
        void SetUp() override {
            std::string vecAsStr;
            this->list->append(list, (void*)hello.c_str());
            ASSERT_EQ(std::string{"[ Hello ]"}, (vecAsStr = CVectorTest::getVectorAsSingleString(list))) << "First element not correct! (" << vecAsStr << ")";
            EXPECT_EQ(2, this->list->_allocated_size) << "Allocated size #1 unexpected (`" << this->list->_allocated_size << "` is not `" << 2 << "`";

            this->list->append(list, (void*)world.c_str());
            ASSERT_EQ(std::string{"[ Hello World ]"}, (vecAsStr = CVectorTest::getVectorAsSingleString(list))) << "Second element not correct! (" << vecAsStr << ")";
            EXPECT_EQ(4, this->list->_allocated_size) << "Allocated size #2 unexpected (`" << this->list->_allocated_size << "` is not `" << 4 << "`";

            this->list->append(list, (void*)exclaim.c_str());
            ASSERT_EQ(std::string{"[ Hello World ! ]"}, (vecAsStr = CVectorTest::getVectorAsSingleString(list))) << "Third element not correct! (" << vecAsStr << ")";
            EXPECT_EQ(8, this->list->_allocated_size) << "Allocated size #3 unexpected (`" << this->list->_allocated_size << "` is not `" << 8 << "`";
        }

    protected:
        CVectorTest() {
            this->list = create_String_Vector(2, false);
        }

        ~CVectorTest() override {
            this->list->freeThis(this->list);
            this->list = nullptr;
        }

        // Methods
    static std::string getVectorAsSingleString(Vector* list){
            std::string vectorString{"[ "};
            for (int i = 0; i < list->_utilized_size; i++) {
                vectorString += std::string{static_cast<char *>(list->get(list, i))} + " ";
            }
            return vectorString + "]";
        }

        // Members
        Vector* list;
        std::string hello{"Hello"};
        std::string world{"World"};
        std::string fair{"Fair"};
        std::string smol{"Smoldyn"};
        std::string exclaim{"!"};
        std::string huh{"HUH"};
};

TEST_F(CVectorTest, Replace) {
    std::string vecAsStr;
    char* old_value = (char*)this->list->replace(list, 1, (void*)smol.c_str());
    EXPECT_EQ(std::string{"[ Hello Smoldyn ! ]"}, (vecAsStr = CVectorTest::getVectorAsSingleString(this->list))) << "Replacement of second element not correct! (" << vecAsStr << ")";
    EXPECT_EQ(std::string{"World"}, std::string{old_value}) << "Old value not correct! (" << std::string{old_value} << ")";
    EXPECT_EQ(8, this->list->_allocated_size) << "Allocated size unexpected (`" << this->list->_allocated_size << "` is not `" << 8 << "`";
}

TEST_F(CVectorTest, Insert) {
    std::string vecAsStr;
    this->list->insert(this->list, 1, (void*)this->fair.c_str());
    EXPECT_EQ(std::string{"[ Hello Fair World ! ]"}, (vecAsStr = CVectorTest::getVectorAsSingleString(this->list))) << "Insertion of element not correct! (" << vecAsStr << ")";
    EXPECT_EQ(8, this->list->_allocated_size) << "Allocated size unexpected (`" << this->list->_allocated_size << "` is not `" << 8 << "`";
}

TEST_F(CVectorTest, AppendThroughInsert) {
    std::string vecAsStr;
    this->list->insert(this->list, this->list->_utilized_size, (void*)this->huh.c_str());
    EXPECT_EQ(std::string{"[ Hello World ! HUH ]"}, (vecAsStr = CVectorTest::getVectorAsSingleString(list))) << "Insert-appended element not correct! (" << vecAsStr << ")";
    EXPECT_EQ(8, this->list->_allocated_size) << "Allocated size unexpected (`" << this->list->_allocated_size << "` is not `" << 8 << "`";
}

TEST_F(CVectorTest, Removal) {
    std::string vecAsStr;
    this->list->remove(this->list, 0);
    this->list->remove(this->list, this->list->_utilized_size - 1);
    EXPECT_EQ(std::string{"[ World ]"}, (vecAsStr = CVectorTest::getVectorAsSingleString(list))) << "Removed element not correct! (" << vecAsStr << ")";
    EXPECT_EQ(2, this->list->_allocated_size) << "Allocated size unexpected (`" << this->list->_allocated_size << "` is not `" << 2 << "`";
}

TEST_F(CVectorTest, Clear) {
    std::string vecAsStr;
    this->list->clear(this->list);
    EXPECT_EQ(std::string{"[ ]"}, (vecAsStr = CVectorTest::getVectorAsSingleString(list))) << "Removed element not correct! (Found: `" << vecAsStr << "`)";
    EXPECT_EQ(2, this->list->_allocated_size) << "Allocated size unexpected (`" << this->list->_allocated_size << "` is not `" << 2 << "`";
}