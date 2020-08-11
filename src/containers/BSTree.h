#pragma once

#include "Array.h"
#include "Common.h"

namespace Containers
{
    // WAVL tree is used here
    template<typename T>
    class BSTree
    {
    private:
        bool (*lessThan)(const T& a, const T& b);

        struct Node
        {
            T value;
            typename Array<Node>::Iterator right;
            typename Array<Node>::Iterator left;
            i32 height = 0;

            Node(const T& val) : value(val) {};

            typename Array<Node>::Iterator& getChild(bool isRight)
            {
                return isRight ? right : left;
            }
        };

        typedef typename Array<Node>::Iterator NodeIterator;

        NodeIterator root;
        NodeIterator top;

        Array<Node> data;
        Array<NodeIterator> emptySlots;

        Array<NodeIterator*> searchPath;

        i32 performSearch(const T& elem, bool shouldReturnIfFound,
                          NodeIterator* searchRoot = nullptr, i32 startingIndex = 0)
        {
            i32 requiredSize = root == nullptr ? 1 : root->height + 2;

            while (requiredSize > searchPath.Size())
            {
                searchPath.Insert(nullptr);
            }

            if (searchRoot == nullptr)
            {
                searchRoot = &root;
            }

            NodeIterator currentNode = *searchRoot;
            i32 i = startingIndex;
            searchPath[startingIndex] = searchRoot;

            while (currentNode != nullptr)
            {
                if (lessThan(elem, currentNode->value))
                {
                    searchPath[++i] = &(currentNode->left);
                    currentNode = currentNode->left;
                }
                else if (!shouldReturnIfFound || lessThan(currentNode->value, elem))
                {
                    searchPath[++i] = &(currentNode->right);
                    currentNode = currentNode->right;
                }
                else
                {
                    return i;
                }
            }

            return i;
        }


        void rotateTree(NodeIterator* currentNode, bool isRotateRight)
        {
            NodeIterator& parent = *currentNode;
            NodeIterator tmp = parent->getChild(!isRotateRight);
            parent->getChild(!isRotateRight) = tmp->getChild(isRotateRight);
            tmp->getChild(isRotateRight) = parent;
            parent = tmp;
        }


        bool tryBalanceTreeOnInsertion(i32 currentIndex, bool isRightChild)
        {
            bool haveSibling = (*searchPath[currentIndex - 1])->getChild(!isRightChild) == nullptr;
            i32 sibHeightDif = haveSibling ? (*searchPath[currentIndex - 1])->height + 1 :
                    (*searchPath[currentIndex - 1])->height - (*searchPath[currentIndex - 1])->getChild(!isRightChild)->height;

            if (sibHeightDif == 1)
            {
                (*searchPath[currentIndex - 1])->height += 1;
                return false;
            }
            else
            {
                if ((*searchPath[currentIndex])->getChild(!isRightChild) == nullptr ||
                    (*searchPath[currentIndex])->height - (*searchPath[currentIndex])->getChild(!isRightChild)->height == 2)
                {
                    (*searchPath[currentIndex - 1])->height -= 1;
                    rotateTree(searchPath[currentIndex - 1], !isRightChild);
                }
                else
                {
                    (*searchPath[currentIndex])->getChild(!isRightChild)->height += 1;
                    (*searchPath[currentIndex])->height -= 1;
                    (*searchPath[currentIndex - 1])->height -= 1;
                    rotateTree(searchPath[currentIndex], isRightChild);
                    rotateTree(searchPath[currentIndex - 1], !isRightChild);
                }
                return true;
            }
        }


        bool tryBalanceTreeOnDeletion(i32 currentIndex, bool isRightChild)
        {
            NodeIterator& sibling = (*searchPath[currentIndex - 1])->getChild(!isRightChild);
            i32 sibHeightDif = sibling != nullptr ? (*searchPath[currentIndex - 1])->height + 1 :
                    (*searchPath[currentIndex - 1])->height - sibling->height;

            if (sibHeightDif == 2)
            {
                (*searchPath[currentIndex - 1])->height -= 1;
                return false;
            }
            else if (sibling != nullptr &&
                     sibling->height - (sibling->right != nullptr ? sibling->right->height : 0) == 2 &&
                     sibling->height - (sibling->left != nullptr ? sibling->left->height : 0) == 2)
            {
                (*searchPath[currentIndex - 1])->height -= 1;
                sibling->height -= 1;
                return false;
            }
            else
            {
                NodeIterator& niece = sibling->getChild(isRightChild);
                NodeIterator& nephew = sibling->getChild(!isRightChild);
                
                if (sibling->height - (nephew == nullptr ? 0 : nephew->height) == 1)
                {
                    (*searchPath[currentIndex - 1])->height -= sibling->height - (niece == nullptr ? 0 : niece->height);
                    sibling->height += 1;
                    rotateTree(searchPath[currentIndex - 1], isRightChild);
                    return true;
                }
                else
                {
                    niece->height += 2;
                    (*searchPath[currentIndex - 1])->height -= 2;
                    sibling->height -= 1;
                    rotateTree(&sibling, !isRightChild);
                    rotateTree(searchPath[currentIndex - 1], isRightChild);
                    return true;
                }
            }
        }


        NodeIterator physicInsert(const T& elem)
        {
            if (!emptySlots.Size())
            {
                data.Insert(Node(elem));
                return NodeIterator(&data, data.Size() - 1);
            }

            NodeIterator wantedNode = emptySlots.Back();
            emptySlots.Remove(1);

            *wantedNode = Node(elem);
            return wantedNode;
        }


        void logicInsert(NodeIterator elem)
        {
            i32 searchIndex = performSearch(elem->value, false);
            *searchPath[searchIndex] = elem;

            while (searchIndex != 0 && (*searchPath[searchIndex - 1])->height - (*searchPath[searchIndex])->height == 0 &&
                   !tryBalanceTreeOnInsertion(searchIndex, (*searchPath[searchIndex - 1])->right == *searchPath[searchIndex]))
            {
                searchIndex -= 1;
            }
        }


        void physicRemove(NodeIterator elem)
        {
            emptySlots.Insert(elem);
        }


        void logicRemove(NodeIterator elem)
        {
            assert(elem != nullptr);

            i32 nodeToDeleteIndex = performSearch(elem->value, true);
            while (*searchPath[nodeToDeleteIndex] != elem)
            {
                assert(*searchPath[nodeToDeleteIndex] != nullptr);
                nodeToDeleteIndex = performSearch(elem->value, true, &(*searchPath[nodeToDeleteIndex])->right, nodeToDeleteIndex + 1);
            }

            i32 currentIndex;
            i32 currentHeight;

            if ((*searchPath[nodeToDeleteIndex])->left != nullptr)
            {
                currentIndex = performSearch(elem->value, false, &(*searchPath[nodeToDeleteIndex])->left, nodeToDeleteIndex + 1) - 1;
                NodeIterator& replaceNode = *searchPath[currentIndex];
                NodeIterator replaceChild = replaceNode->left;

                replaceNode->left = (*searchPath[nodeToDeleteIndex])->left;
                replaceNode->right = (*searchPath[nodeToDeleteIndex])->right;
                *searchPath[nodeToDeleteIndex] = replaceNode;
                searchPath[nodeToDeleteIndex] = &replaceNode;

                replaceNode = replaceChild;
                searchPath[currentIndex] = &replaceChild;

                currentHeight = replaceChild == nullptr ? 0 : replaceChild->height;
            }
            else
            {
                currentIndex = nodeToDeleteIndex;

                NodeIterator tmp = (*searchPath[nodeToDeleteIndex])->right;
                *searchPath[nodeToDeleteIndex] = tmp;
                searchPath[nodeToDeleteIndex] = &tmp;

                currentHeight = tmp == nullptr ? 0 : tmp->height;
            }

            while (currentIndex != 0 && (*searchPath[currentIndex - 1])->height - currentHeight == 3 &&
                   !tryBalanceTreeOnDeletion(currentIndex, (*searchPath[currentIndex - 1])->right == *searchPath[currentIndex]))
            {
                currentIndex -= 1;
            }

            top = root;
            while (top->right != nullptr)
            {
                top = top->right;
            }
        }

    public:
        typedef NodeIterator Iterator;

        explicit BSTree(bool (*lessFunction)(const T&, const T&) = (&defaultLess<T>)) : lessThan(lessFunction) {};

        void Remove(NodeIterator& elem)
        {
            logicRemove(elem);
            physicRemove(elem);
        }

        Iterator Insert(const T& elem)
        {
            Iterator insertedNode = physicInsert(elem);
            logicInsert(insertedNode);

            if (top == nullptr || lessThan(top->value, insertedNode->value))
            {
                top = insertedNode;
            }

            return insertedNode;
        }

        const T& Top()
        {
            return top->value;
        }
    };
}
