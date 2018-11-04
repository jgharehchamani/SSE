#include "AVLTree.h"

AVLTree::AVLTree(int maxSize, bytes<Key> key) : rd(), mt(rd()), dis(0, (pow(2, floor(log2(maxSize / Z)) + 1) - 1) / 2) {
    oram = new ORAM(maxSize, key);
}

AVLTree::~AVLTree() {
    delete oram;
}

// A utility function to get height of the tree

int AVLTree::height(Bid key, int& leaf) {
    if (key == 0)
        return 0;
    Node* node = oram->ReadNode(key, leaf, leaf);
    return node->height;
}

// A utility function to get maximum of two integers

int AVLTree::max(int a, int b) {
    return (a > b) ? a : b;
}

/* Helper function that allocates a new node with the given key and
   NULL left and right pointers. */
Node* AVLTree::newNode(Bid key, string value) {
    Node* node = new Node();
    node->key = key;
    std::fill(node->value.begin(), node->value.end(), 0);
    std::copy(value.begin(), value.end(), node->value.begin());
    node->leftID = 0;
    node->rightID = 0;
    node->pos = RandomPath();
    node->height = 1; // new node is initially added at leaf
    return node;
}

// A utility function to right rotate subtree rooted with y
// See the diagram given above.

Node* AVLTree::rightRotate(Node* y) {
    Node* x = oram->ReadNode(y->leftID);
    Node* T2;
    if (x->rightID == 0) {
        T2 = newNode(0, "");
    } else {
        T2 = oram->ReadNode(x->rightID);
    }

    // Perform rotation
    x->rightID = y->key;
    x->rightPos = y->pos;
    y->leftID = T2->key;
    y->leftPos = T2->pos;

    // Update heights
    y->height = max(height(y->leftID, y->leftPos), height(y->rightID, y->rightPos)) + 1;
    oram->WriteNode(y->key, y);
    x->height = max(height(x->leftID, x->leftPos), height(x->rightID, x->rightPos)) + 1;
    oram->WriteNode(x->key, x);
    // Return new root

    return x;
}

// A utility function to left rotate subtree rooted with x
// See the diagram given above.

Node* AVLTree::leftRotate(Node* x) {
    Node* y = oram->ReadNode(x->rightID);
    Node* T2;
    if (y->leftID == 0) {
        T2 = newNode(0, "");
    } else {
        T2 = oram->ReadNode(y->leftID);
    }


    // Perform rotation
    y->leftID = x->key;
    y->leftPos = x->pos;
    x->rightID = T2->key;
    x->rightPos = T2->pos;

    // Update heights
    x->height = max(height(x->leftID, x->leftPos), height(x->rightID, x->rightPos)) + 1;
    oram->WriteNode(x->key, x);
    y->height = max(height(y->leftID, y->leftPos), height(y->rightID, y->rightPos)) + 1;
    oram->WriteNode(y->key, y);
    // Return new root
    return y;
}

// Get Balance factor of node N

int AVLTree::getBalance(Node* N) {
    if (N == NULL)
        return 0;
    return height(N->leftID, N->leftPos) - height(N->rightID, N->rightPos);
}

Bid AVLTree::insert(Bid rootKey, int& pos, Bid key, string value) {
    /* 1. Perform the normal BST rotation */
    if (rootKey == 0) {
        Node* nnode = newNode(key, value);
        pos = oram->WriteNode(key, nnode);
        return nnode->key;
    }
    Node* node = oram->ReadNode(rootKey, pos, pos);
    if (key < node->key) {
        node->leftID = insert(node->leftID, node->leftPos, key, value);
    } else if (key > node->key) {
        node->rightID = insert(node->rightID, node->rightPos, key, value);
    } else {
        std::fill(node->value.begin(), node->value.end(), 0);
        std::copy(value.begin(), value.end(), node->value.begin());
        oram->WriteNode(rootKey, node);
        return node->key;
    }

    /* 2. Update height of this ancestor node */
    node->height = max(height(node->leftID, node->leftPos), height(node->rightID, node->rightPos)) + 1;

    /* 3. Get the balance factor of this ancestor node to check whether
       this node became unbalanced */
    int balance = getBalance(node);

    // If this node becomes unbalanced, then there are 4 cases

    // Left Left Case
    if (balance > 1 && key < oram->ReadNode(node->leftID)->key) {
        Node* res = rightRotate(node);
        pos = res->pos;
        return res->key;
    }

    // Right Right Case
    if (balance < -1 && key > oram->ReadNode(node->rightID)->key) {
        Node* res = leftRotate(node);
        pos = res->pos;
        return res->key;
    }

    // Left Right Case
    if (balance > 1 && key > oram->ReadNode(node->leftID)->key) {
        Node* res = leftRotate(oram->ReadNode(node->leftID));
        node->leftID = res->key;
        node->leftPos = res->pos;
        oram->WriteNode(node->key, node);
        Node* res2 = rightRotate(node);
        pos = res2->pos;
        return res2->key;
    }

    // Right Left Case
    if (balance < -1 && key < oram->ReadNode(node->rightID)->key) {
        auto res = rightRotate(oram->ReadNode(node->rightID));
        node->rightID = res->key;
        node->rightPos = res->pos;
        oram->WriteNode(node->key, node);
        auto res2 = leftRotate(node);
        pos = res2->pos;
        return res2->key;
    }

    /* return the (unchanged) node pointer */
    oram->WriteNode(node->key, node);
    return node->key;
}

/**
 * a recursive search function which traverse binary tree to find the target node
 */
Node* AVLTree::search(Node* head, Bid key) {
    if (head == NULL || head->key == 0)
        return head;
    head = oram->ReadNode(head->key, head->pos, head->pos);
    if (head->key > key) {
        return search(oram->ReadNode(head->leftID, head->leftPos, head->leftPos), key);
    } else if (head->key < key) {
        return search(oram->ReadNode(head->rightID, head->rightPos, head->rightPos), key);
    } else
        return head;
}

/**
 * a recursive search function which traverse binary tree to find the target node
 */
void AVLTree::batchSearch(Node* head, vector<Bid> keys, vector<Node*>* results) {
    if (head == NULL || head->key == 0) {
        return;
    }
    head = oram->ReadNode(head->key, head->pos, head->pos);
    bool getLeft = false, getRight = false;
    vector<Bid> leftkeys,rightkeys;
    for (Bid bid : keys) {
        if (head->key > bid) {
            getLeft = true;
            leftkeys.push_back(bid);
        }
        if (head->key < bid) {
            getRight = true;
            rightkeys.push_back(bid);
        }
        if (head->key == bid) {
            results->push_back(head);
        }
    }
    if (getLeft) {
        batchSearch(oram->ReadNode(head->leftID, head->leftPos, head->leftPos), leftkeys, results);
    }
    if (getRight) {
        batchSearch(oram->ReadNode(head->rightID, head->rightPos, head->rightPos), rightkeys, results);
    }
}

void AVLTree::printTree(Node* root, int indent) {
    if (root != 0 && root->key != 0) {
        root = oram->ReadNode(root->key, root->pos, root->pos);
        if (root->leftID != 0)
            printTree(oram->ReadNode(root->leftID, root->leftPos, root->leftPos), indent + 4);
        if (indent > 0)
            cout << setw(indent) << " ";
        string value;
        value.assign(root->value.begin(), root->value.end());
        cout << root->key << ":" << value.c_str() << ":" << root->pos << ":" << root->leftID << ":" << root->leftPos << ":" << root->rightID << ":" << root->rightPos << endl;
        if (root->rightID != 0)
            printTree(oram->ReadNode(root->rightID, root->rightPos, root->rightPos), indent + 4);

    }
}

/*
 * before executing each operation, this function should be called with proper arguments
 */
void AVLTree::startOperation(bool batchWrite) {
    oram->start(batchWrite);
}

/*
 * after executing each operation, this function should be called with proper arguments
 */
void AVLTree::finishOperation(bool find, Bid& rootKey, int& rootPos) {
    oram->finilize(find, rootKey, rootPos);
}

int AVLTree::RandomPath() {
    int val = dis(mt);
    return val;
}