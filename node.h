#include <stdio.h>
#include <stdlib.h>

//
// Created by ltabstergo on 2020-02-03.
//
struct HistoryNode *header;
struct HistoryNode *tail;

struct HistoryNode {
    struct HistoryNode *previous;
    char **input;
    int argc;
    struct HistoryNode *next;
};

void setNext(struct HistoryNode *a, struct HistoryNode *b) {
    if (a != NULL && b != NULL) {
        a->next = b;
        b->previous = a;
    }
}

void setPrevious(struct HistoryNode *a, struct HistoryNode *b) {
    if (a != NULL && b != NULL) {
        a->next = b;
        b->previous = a;
    }
}

void addLast(char **in, int argc) {
    struct HistoryNode *node = (struct HistoryNode *) malloc(sizeof(struct HistoryNode));
    node->input = in;
    node->argc = argc;
    setNext(tail, node);
    tail = node;
    node->next = NULL;
}

void emptyList() {
    struct HistoryNode *cur = tail;
    struct HistoryNode *temp;
    while (cur != NULL) {
        temp = cur->previous;
        free(cur);
        cur = temp;
    }
}

