#ifndef __TEST_ITEM_CONTEXT__
#define __TEST_ITEM_CONTEXT__

#include <Constants.h>
#include <JVector.h>
#include "Item.h"
#include "Dungeon.h"
#include "FileParse.h"



/*#######
##
## CONTEXT
##
#######*/
struct TestItemCtx 
{
    JLinkList <CItemDef> *m_llItemDefs;
    CItemDef *ItemDef;
    JResult Success;
};

#endif // __TEST_ITEM_CONTEXT__
