//
// Created by AivenLau on 2016/12/12.
//

#include "MyFrame.h"

MyFrame::MyFrame() : data(NULL), nLen(0),bKeyFranme(false)
{

}

MyFrame::~MyFrame() {
    if (data != NULL) {
        delete[]data;
    }
    nLen = 0;
}

void MyFrame::Release()
{
    if (data != NULL) {
        delete[]data;
        data = NULL;
    }
    nLen = 0;
}