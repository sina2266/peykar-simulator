//
// Created by Logan Drescher on 3/19/25.
//

#include <gtest/gtest.h>
#include "ComplexDataStructures/LinkedList.h"

class CLinkedListTest : public testing::Test {
    public:
        void SetUp() override {
            std::string llAsStr;
            this->list->append(list, (void*)hello.c_str());
            ASSERT_EQ(std::string{"Hello->0"}, (llAsStr = CLinkedListTest::getLinkedListAsSingleString(list))) << "First element not correct! (" << llAsStr << ")";

            this->list->append(list, (void*)world.c_str());
            ASSERT_EQ(std::string{"Hello->World->0"}, (llAsStr = CLinkedListTest::getLinkedListAsSingleString(list))) << "Second element not correct! (" << llAsStr << ")";

            this->list->append(list, (void*)exclaim.c_str());
            ASSERT_EQ(std::string{"Hello->World->!->0"}, (llAsStr = CLinkedListTest::getLinkedListAsSingleString(list))) << "Third element not correct! (" << llAsStr << ")";
        }

    protected:
        CLinkedListTest() {
            this->list = create_String_LinkedList(false);
        }

        ~CLinkedListTest() override {
            this->list->freeThis(this->list);
            this->list = nullptr;
        }

        // Methods
        static std::string getLinkedListAsSingleString(LinkedList* list){
            if (list->_size == 0) return "0";
            std::string vectorString{};
            LinkedList_Node* currentNode = list->_head;
            while (currentNode) {
                vectorString += std::string{static_cast<char *>(currentNode->data)} + "->";
                currentNode = currentNode->next;
            }
            return vectorString + "0";
        }

        // Members
        LinkedList* list;
        std::string hello{"Hello"};
        std::string world{"World"};
        std::string fair{"Fair"};
        std::string smol{"Smoldyn"};
        std::string exclaim{"!"};
        std::string huh{"HUH"};
};

TEST_F(CLinkedListTest, Replace) {
    std::string vecAsStr;
    char* old_value = (char*)this->list->replace(list, 1, (void*)smol.c_str());
    EXPECT_EQ(std::string{"Hello->Smoldyn->!->0"}, (vecAsStr = CLinkedListTest::getLinkedListAsSingleString(this->list))) << "Replacement of second element not correct! (" << vecAsStr << ")";
    EXPECT_EQ(std::string{"World"}, std::string{old_value}) << "Old value not correct! (" << std::string{old_value} << ")";
}

TEST_F(CLinkedListTest, Insert) {
    std::string vecAsStr;
    this->list->insert(this->list, 1, (void*)this->fair.c_str());
    EXPECT_EQ(std::string{"Hello->Fair->World->!->0"}, (vecAsStr = CLinkedListTest::getLinkedListAsSingleString(this->list))) << "Insertion of element not correct! (" << vecAsStr << ")";
}

TEST_F(CLinkedListTest, AppendThroughInsert) {
    std::string vecAsStr;
    this->list->insert(this->list, this->list->_size, (void*)this->huh.c_str());
    EXPECT_EQ(std::string{"Hello->World->!->HUH->0"}, (vecAsStr = CLinkedListTest::getLinkedListAsSingleString(list))) << "Insert-appended element not correct! (" << vecAsStr << ")";
}

TEST_F(CLinkedListTest, Removal) {
    std::string vecAsStr;
    this->list->remove(this->list, 0);
    this->list->remove(this->list, this->list->_size - 1);
    EXPECT_EQ(std::string{"World->0"}, (vecAsStr = CLinkedListTest::getLinkedListAsSingleString(list))) << "Removed element not correct! (" << vecAsStr << ")";
}

TEST_F(CLinkedListTest, Clear) {
    std::string vecAsStr;
    this->list->clear(this->list);
    EXPECT_EQ(std::string{"0"}, (vecAsStr = CLinkedListTest::getLinkedListAsSingleString(list))) << "Removed element not correct! (Found: `" << vecAsStr << "`)";
}