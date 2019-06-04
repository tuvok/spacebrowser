#ifndef VIEWHANDLER_TEST_H_
#define VIEWHANDLER_TEST_H_

#include <QObject>
#include <QQuickItem>
#include <QtTest/QtTest>

#include <test/ViewHandler_test_mock.h>

#include <ViewHandler.h>

#include <iostream>
#include <map>

class ViewHandler_test: public QObject
{
    Q_OBJECT

private:
    void fillWithThree(ViewHandler& vh)
    {
        using namespace testing;

        /// Make sure model is empty
        QCOMPARE(vh.tabsModel.rowCount(), 0);
        QCOMPARE(vh.flatModel.rowCount(), 0);
        QCOMPARE(vh.views2.size(), 0);

        EXPECT_CALL(vh.tabsDb, createTab()).Times(3)
            .WillOnce(Return(1)).WillOnce(Return(2)).WillOnce(Return(3));
        EXPECT_CALL(vh.tabsDb, setParent(1, 0));
        EXPECT_CALL(vh.tabsDb, setParent(2, 0));
        EXPECT_CALL(vh.tabsDb, setParent(3, 0));

        vh.createTab(0);
        QCOMPARE(vh.tabsModel.rowCount(), 1);
        QCOMPARE(vh.flatModel.rowCount(), 1);
        QCOMPARE(vh.views2.size(), 1);

        vh.createTab(0);
        QCOMPARE(vh.tabsModel.rowCount(), 2);
        QCOMPARE(vh.flatModel.rowCount(), 2);
        QCOMPARE(vh.views2.size(), 2);

        vh.createTab(0);
        QCOMPARE(vh.tabsModel.rowCount(), 3);
        QCOMPARE(vh.flatModel.rowCount(), 3);
        QCOMPARE(vh.views2.size(), 3);

        /// Check internals of flatModel
        QCOMPARE(vh.flatModel.toSource.size(), 3);
        QCOMPARE(vh.flatModel.fromSource.size(), 3);
        QCOMPARE(vh.flatModel.viewId2ModelId.size(), 3);

        QCOMPARE(vh.flatModel.toSource[0]->data().toInt(), 1);
        QCOMPARE(vh.flatModel.toSource[1]->data().toInt(), 2);
        QCOMPARE(vh.flatModel.toSource[2]->data().toInt(), 3);

        for (unsigned int i = 0; i < vh.flatModel.toSource.size(); ++i)
        {
            auto& item = vh.flatModel.toSource[i];
            QCOMPARE(vh.flatModel.fromSource[item], i);
        }

        QCOMPARE(vh.flatModel.viewId2ModelId[1], 0);
        QCOMPARE(vh.flatModel.viewId2ModelId[2], 1);
        QCOMPARE(vh.flatModel.viewId2ModelId[3], 2);
    }

    void fillWithThreePlusChildren(ViewHandler& vh, int parentId)
    {
        using namespace testing;

        /// Make sure model is empty
        QCOMPARE(vh.tabsModel.rowCount(), 0);
        QCOMPARE(vh.flatModel.rowCount(), 0);
        QCOMPARE(vh.views2.size(), 0);

        EXPECT_CALL(vh.tabsDb, createTab()).Times(6)
            .WillOnce(Return(1)).WillOnce(Return(2)).WillOnce(Return(3))
            .WillOnce(Return(4)).WillOnce(Return(5)).WillOnce(Return(6));
        EXPECT_CALL(vh.tabsDb, setParent(1, 0));
        EXPECT_CALL(vh.tabsDb, setParent(2, 0));
        EXPECT_CALL(vh.tabsDb, setParent(3, 0));
        EXPECT_CALL(vh.tabsDb, setParent(4, parentId));
        EXPECT_CALL(vh.tabsDb, setParent(5, parentId));
        EXPECT_CALL(vh.tabsDb, setParent(6, parentId));

        vh.createTab(0);
        QCOMPARE(vh.tabsModel.rowCount(), 1);
        QCOMPARE(vh.flatModel.rowCount(), 1);
        QCOMPARE(vh.views2.size(), 1);

        vh.createTab(0);
        QCOMPARE(vh.tabsModel.rowCount(), 2);
        QCOMPARE(vh.flatModel.rowCount(), 2);
        QCOMPARE(vh.views2.size(), 2);

        vh.createTab(0);
        QCOMPARE(vh.tabsModel.rowCount(), 3);
        QCOMPARE(vh.flatModel.rowCount(), 3);
        QCOMPARE(vh.views2.size(), 3);

        vh.createTab(parentId);
        QCOMPARE(vh.tabsModel.rowCount(), 3);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(parentId-1)->rowCount(), 1);
        QCOMPARE(vh.flatModel.rowCount(), 4);
        QCOMPARE(vh.views2.size(), 4);

        vh.createTab(parentId);
        QCOMPARE(vh.tabsModel.rowCount(), 3);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(parentId-1)->rowCount(), 2);
        QCOMPARE(vh.flatModel.rowCount(), 5);
        QCOMPARE(vh.views2.size(), 5);

        vh.createTab(parentId);
        QCOMPARE(vh.tabsModel.rowCount(), 3);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(parentId-1)->rowCount(), 3);
        QCOMPARE(vh.flatModel.rowCount(), 6);
        QCOMPARE(vh.views2.size(), 6);


        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(0))->getId(), 1);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(1))->getId(), 2);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(2))->getId(), 3);

        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(parentId-1)->child(0))->getId(), 4);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(parentId-1)->child(1))->getId(), 5);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(parentId-1)->child(2))->getId(), 6);


        QCOMPARE(vh.flatModel.getModelId(1), 0);
        switch(parentId)
        {
            case 1: {
                QCOMPARE(vh.flatModel.getModelId(2), 4);
                QCOMPARE(vh.flatModel.getModelId(3), 5);
                QCOMPARE(vh.flatModel.getModelId(4), 1);
                QCOMPARE(vh.flatModel.getModelId(5), 2);
                QCOMPARE(vh.flatModel.getModelId(6), 3);
            } break;

            case 2: {
                QCOMPARE(vh.flatModel.getModelId(2), 1);
                QCOMPARE(vh.flatModel.getModelId(3), 5);
                QCOMPARE(vh.flatModel.getModelId(4), 2);
                QCOMPARE(vh.flatModel.getModelId(5), 3);
                QCOMPARE(vh.flatModel.getModelId(6), 4);
            } break;

            case 3: {
                QCOMPARE(vh.flatModel.getModelId(2), 1);
                QCOMPARE(vh.flatModel.getModelId(3), 2);
                QCOMPARE(vh.flatModel.getModelId(4), 3);
                QCOMPARE(vh.flatModel.getModelId(5), 4);
                QCOMPARE(vh.flatModel.getModelId(6), 5);
            } break;
        }

        QCOMPARE(vh.views2.count(1), 1);
        QCOMPARE(vh.views2.count(2), 1);
        QCOMPARE(vh.views2.count(3), 1);
        QCOMPARE(vh.views2.count(4), 1);
        QCOMPARE(vh.views2.count(5), 1);
        QCOMPARE(vh.views2.count(6), 1);

        QCOMPARE(vh.views2.at(1).tabData->getId(), 1);
        QCOMPARE(vh.views2.at(2).tabData->getId(), 2);
        QCOMPARE(vh.views2.at(3).tabData->getId(), 3);
        QCOMPARE(vh.views2.at(4).tabData->getId(), 4);
        QCOMPARE(vh.views2.at(5).tabData->getId(), 5);
        QCOMPARE(vh.views2.at(6).tabData->getId(), 6);

        /// Check internals of flatModel
        QCOMPARE(vh.flatModel.toSource.size(), 6);
        QCOMPARE(vh.flatModel.fromSource.size(), 6);
        QCOMPARE(vh.flatModel.viewId2ModelId.size(), 6);

        for (unsigned int i = 0; i < vh.flatModel.toSource.size(); ++i)
        {
            auto& item = vh.flatModel.toSource[i];
            QCOMPARE(vh.flatModel.fromSource[item], i);
        }

        switch(parentId)
        {
            case 1: {
                QCOMPARE(vh.flatModel.toSource[0]->data().toInt(), 1);
                QCOMPARE(vh.flatModel.toSource[1]->data().toInt(), 4);
                QCOMPARE(vh.flatModel.toSource[2]->data().toInt(), 5);
                QCOMPARE(vh.flatModel.toSource[3]->data().toInt(), 6);
                QCOMPARE(vh.flatModel.toSource[4]->data().toInt(), 2);
                QCOMPARE(vh.flatModel.toSource[5]->data().toInt(), 3);

                QCOMPARE(vh.flatModel.viewId2ModelId[1], 0);
                QCOMPARE(vh.flatModel.viewId2ModelId[2], 4);
                QCOMPARE(vh.flatModel.viewId2ModelId[3], 5);
                QCOMPARE(vh.flatModel.viewId2ModelId[4], 1);
                QCOMPARE(vh.flatModel.viewId2ModelId[5], 2);
                QCOMPARE(vh.flatModel.viewId2ModelId[6], 3);
            } break;

            case 2: {
                QCOMPARE(vh.flatModel.toSource[0]->data().toInt(), 1);
                QCOMPARE(vh.flatModel.toSource[1]->data().toInt(), 2);
                QCOMPARE(vh.flatModel.toSource[2]->data().toInt(), 4);
                QCOMPARE(vh.flatModel.toSource[3]->data().toInt(), 5);
                QCOMPARE(vh.flatModel.toSource[4]->data().toInt(), 6);
                QCOMPARE(vh.flatModel.toSource[5]->data().toInt(), 3);

                QCOMPARE(vh.flatModel.viewId2ModelId[1], 0);
                QCOMPARE(vh.flatModel.viewId2ModelId[2], 1);
                QCOMPARE(vh.flatModel.viewId2ModelId[3], 5);
                QCOMPARE(vh.flatModel.viewId2ModelId[4], 2);
                QCOMPARE(vh.flatModel.viewId2ModelId[5], 3);
                QCOMPARE(vh.flatModel.viewId2ModelId[6], 4);
            } break;

            case 3: {
                QCOMPARE(vh.flatModel.toSource[0]->data().toInt(), 1);
                QCOMPARE(vh.flatModel.toSource[1]->data().toInt(), 2);
                QCOMPARE(vh.flatModel.toSource[2]->data().toInt(), 3);
                QCOMPARE(vh.flatModel.toSource[3]->data().toInt(), 4);
                QCOMPARE(vh.flatModel.toSource[4]->data().toInt(), 5);
                QCOMPARE(vh.flatModel.toSource[5]->data().toInt(), 6);

                QCOMPARE(vh.flatModel.viewId2ModelId[1], 0);
                QCOMPARE(vh.flatModel.viewId2ModelId[2], 1);
                QCOMPARE(vh.flatModel.viewId2ModelId[3], 2);
                QCOMPARE(vh.flatModel.viewId2ModelId[4], 3);
                QCOMPARE(vh.flatModel.viewId2ModelId[5], 4);
                QCOMPARE(vh.flatModel.viewId2ModelId[6], 5);
            } break;
        }
    }


    /// |
    /// \_1
    ///  \_4
    ///   \_7
    ///   \_16
    ///   \_17
    ///  \_10
    ///   \_18
    ///   \_19
    ///   \_20
    ///  \_11
    ///   \_21
    ///   \_22
    ///   \_23
    /// \_2
    ///  \_5
    ///   \_8
    ///   \_24
    ///   \_25
    ///  \_12
    ///   \_26
    ///   \_27
    ///   \_28
    ///  \_13
    ///   \_29
    ///   \_30
    ///   \_31
    /// \_3
    ///  \_6
    ///   \_9
    ///  \_14
    ///  \_15
    void fillComplexStructure(ViewHandler& vh)
    {
        using namespace testing;

        fillWithThree(vh);

        int i = 3;
        EXPECT_CALL(vh.tabsDb, createTab()).Times(28).WillRepeatedly(Invoke([&i](){ return ++i; }));

        EXPECT_CALL(vh.tabsDb, setParent(4, 1));
        EXPECT_CALL(vh.tabsDb, setParent(5, 2));
        EXPECT_CALL(vh.tabsDb, setParent(6, 3));

        vh.createTab(1);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(0)->rowCount(), 1);
        QCOMPARE(vh.flatModel.rowCount(), 4);
        QCOMPARE(vh.views2.size(), 4);

        vh.createTab(2);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(1)->rowCount(), 1);
        QCOMPARE(vh.flatModel.rowCount(), 5);
        QCOMPARE(vh.views2.size(), 5);

        vh.createTab(3);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(2)->rowCount(), 1);
        QCOMPARE(vh.flatModel.rowCount(), 6);
        QCOMPARE(vh.views2.size(), 6);

        EXPECT_CALL(vh.tabsDb, setParent(7, 4));
        EXPECT_CALL(vh.tabsDb, setParent(8, 5));
        EXPECT_CALL(vh.tabsDb, setParent(9, 6));

        vh.createTab(4);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(0)->child(0)->rowCount(), 1);
        QCOMPARE(vh.flatModel.rowCount(), 7);
        QCOMPARE(vh.views2.size(), 7);

        vh.createTab(5);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(1)->child(0)->rowCount(), 1);
        QCOMPARE(vh.flatModel.rowCount(), 8);
        QCOMPARE(vh.views2.size(), 8);

        vh.createTab(6);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(2)->child(0)->rowCount(), 1);
        QCOMPARE(vh.flatModel.rowCount(), 9);
        QCOMPARE(vh.views2.size(), 9);

        EXPECT_CALL(vh.tabsDb, setParent(10, 1));
        EXPECT_CALL(vh.tabsDb, setParent(11, 1));

        vh.createTab(1);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(0)->rowCount(), 2);
        QCOMPARE(vh.flatModel.rowCount(), 10);
        QCOMPARE(vh.views2.size(), 10);

        vh.createTab(1);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(0)->rowCount(), 3);
        QCOMPARE(vh.flatModel.rowCount(), 11);
        QCOMPARE(vh.views2.size(), 11);

        EXPECT_CALL(vh.tabsDb, setParent(12, 2));
        EXPECT_CALL(vh.tabsDb, setParent(13, 2));

        vh.createTab(2);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(1)->rowCount(), 2);
        QCOMPARE(vh.flatModel.rowCount(), 12);
        QCOMPARE(vh.views2.size(), 12);

        vh.createTab(2);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(1)->rowCount(), 3);
        QCOMPARE(vh.flatModel.rowCount(), 13);
        QCOMPARE(vh.views2.size(), 13);

        EXPECT_CALL(vh.tabsDb, setParent(14, 3));
        EXPECT_CALL(vh.tabsDb, setParent(15, 3));

        vh.createTab(3);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(2)->rowCount(), 2);
        QCOMPARE(vh.flatModel.rowCount(), 14);
        QCOMPARE(vh.views2.size(), 14);

        vh.createTab(3);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(2)->rowCount(), 3);
        QCOMPARE(vh.flatModel.rowCount(), 15);
        QCOMPARE(vh.views2.size(), 15);

        EXPECT_CALL(vh.tabsDb, setParent(16, 4));
        EXPECT_CALL(vh.tabsDb, setParent(17, 4));

        vh.createTab(4);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(0)->child(0)->rowCount(), 2);
        QCOMPARE(vh.flatModel.rowCount(), 16);
        QCOMPARE(vh.views2.size(), 16);

        vh.createTab(4);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(0)->child(0)->rowCount(), 3);
        QCOMPARE(vh.flatModel.rowCount(), 17);
        QCOMPARE(vh.views2.size(), 17);

        EXPECT_CALL(vh.tabsDb, setParent(18, 10));
        EXPECT_CALL(vh.tabsDb, setParent(19, 10));
        EXPECT_CALL(vh.tabsDb, setParent(20, 10));

        vh.createTab(10);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(0)->child(1)->rowCount(), 1);
        QCOMPARE(vh.flatModel.rowCount(), 18);
        QCOMPARE(vh.views2.size(), 18);

        vh.createTab(10);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(0)->child(1)->rowCount(), 2);
        QCOMPARE(vh.flatModel.rowCount(), 19);
        QCOMPARE(vh.views2.size(), 19);

        vh.createTab(10);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(0)->child(1)->rowCount(), 3);
        QCOMPARE(vh.flatModel.rowCount(), 20);
        QCOMPARE(vh.views2.size(), 20);

        EXPECT_CALL(vh.tabsDb, setParent(21, 11));
        EXPECT_CALL(vh.tabsDb, setParent(22, 11));
        EXPECT_CALL(vh.tabsDb, setParent(23, 11));

        vh.createTab(11);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(0)->child(2)->rowCount(), 1);
        QCOMPARE(vh.flatModel.rowCount(), 21);
        QCOMPARE(vh.views2.size(), 21);

        vh.createTab(11);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(0)->child(2)->rowCount(), 2);
        QCOMPARE(vh.flatModel.rowCount(), 22);
        QCOMPARE(vh.views2.size(), 22);

        vh.createTab(11);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(0)->child(2)->rowCount(), 3);
        QCOMPARE(vh.flatModel.rowCount(), 23);
        QCOMPARE(vh.views2.size(), 23);

        EXPECT_CALL(vh.tabsDb, setParent(24, 5));
        EXPECT_CALL(vh.tabsDb, setParent(25, 5));

        vh.createTab(5);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(1)->child(0)->rowCount(), 2);
        QCOMPARE(vh.flatModel.rowCount(), 24);
        QCOMPARE(vh.views2.size(), 24);

        vh.createTab(5);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(1)->child(0)->rowCount(), 3);
        QCOMPARE(vh.flatModel.rowCount(), 25);
        QCOMPARE(vh.views2.size(), 25);

        EXPECT_CALL(vh.tabsDb, setParent(26, 12));
        EXPECT_CALL(vh.tabsDb, setParent(27, 12));
        EXPECT_CALL(vh.tabsDb, setParent(28, 12));

        vh.createTab(12);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(1)->child(1)->rowCount(), 1);
        QCOMPARE(vh.flatModel.rowCount(), 26);
        QCOMPARE(vh.views2.size(), 26);

        vh.createTab(12);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(1)->child(1)->rowCount(), 2);
        QCOMPARE(vh.flatModel.rowCount(), 27);
        QCOMPARE(vh.views2.size(), 27);

        vh.createTab(12);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(1)->child(1)->rowCount(), 3);
        QCOMPARE(vh.flatModel.rowCount(), 28);
        QCOMPARE(vh.views2.size(), 28);

        EXPECT_CALL(vh.tabsDb, setParent(29, 13));
        EXPECT_CALL(vh.tabsDb, setParent(30, 13));
        EXPECT_CALL(vh.tabsDb, setParent(31, 13));

        vh.createTab(13);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(1)->child(2)->rowCount(), 1);
        QCOMPARE(vh.flatModel.rowCount(), 29);
        QCOMPARE(vh.views2.size(), 29);

        vh.createTab(13);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(1)->child(2)->rowCount(), 2);
        QCOMPARE(vh.flatModel.rowCount(), 30);
        QCOMPARE(vh.views2.size(), 30);

        vh.createTab(13);
        QCOMPARE(vh.tabsModel.invisibleRootItem()->child(1)->child(2)->rowCount(), 3);
        QCOMPARE(vh.flatModel.rowCount(), 31);
        QCOMPARE(vh.views2.size(), 31);

        QCOMPARE(vh.tabsModel.rowCount(), 3);
    }

    void checkComplexStructure(ViewHandler& vh, int deleted = -1)
    {
        std::map<int, int> flatId2ViewId =
            {{0, 1}, {1, 4}, {2, 7}, {3, 16}, {4, 17}, {5, 10}, {6, 18}, {7, 19}, {8, 20}, {9, 11},
             {10, 21}, {11, 22}, {12, 23}, {13, 2}, {14, 5}, {15, 8}, {16, 24}, {17, 25}, {18, 12}, {19, 26},
             {20, 27}, {21, 28}, {22, 13}, {23, 29}, {24, 30}, {25, 31}, {26, 3}, {27, 6}, {28, 9}, {29, 14},
             {30, 15}};

        /// Check flat model IDs
        int offset = 0;
        for (int i = 0; i < vh.flatModel.rowCount(); ++i)
        {
            int viewId = ((Tab*)vh.flatModel.toSource.at(i))->getId();

            std::cout << "id: " << i << " viewId: " << viewId << std::endl;

            if (viewId == deleted)
            {
                std::cout << "---> offsetting\n";
                offset = 1;
                continue;
            }

            int mViewId = flatId2ViewId.at(i+offset);

            QCOMPARE(viewId, mViewId);
        }

        /// Check internal mappings in flatModel
        for (unsigned int i = 0; i < vh.flatModel.toSource.size(); ++i)
        {
            auto& item = vh.flatModel.toSource[i];
            QCOMPARE(vh.flatModel.fromSource[item], i);
        }


        /// Check tabsModel structure
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(0))->getId(), 1);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(1))->getId(), 2);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(2))->getId(), 3);


        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(0)->child(0))->getId(), 4);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(0)->child(1))->getId(), 10);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(0)->child(2))->getId(), 11);

        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(1)->child(0))->getId(), 5);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(1)->child(1))->getId(), 26);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(1)->child(2))->getId(), 27);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(1)->child(3))->getId(), 28);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(1)->child(4))->getId(), 13);

        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(2)->child(0))->getId(), 6);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(2)->child(1))->getId(), 14);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(2)->child(2))->getId(), 15);


        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(0)->child(0)->child(0))->getId(), 7);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(0)->child(0)->child(1))->getId(), 16);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(0)->child(0)->child(2))->getId(), 17);

        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(0)->child(1)->child(0))->getId(), 18);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(0)->child(1)->child(1))->getId(), 19);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(0)->child(1)->child(2))->getId(), 20);

        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(0)->child(2)->child(0))->getId(), 21);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(0)->child(2)->child(1))->getId(), 22);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(0)->child(2)->child(2))->getId(), 23);


        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(1)->child(0)->child(0))->getId(), 8);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(1)->child(0)->child(1))->getId(), 24);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(1)->child(0)->child(2))->getId(), 25);

        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(1)->child(4)->child(0))->getId(), 29);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(1)->child(4)->child(1))->getId(), 30);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(1)->child(4)->child(2))->getId(), 31);

    }


private slots:
    void initTestCase()
    {

//        qDebug("Called before everything else.");
    }

    /// Test adding tabs, then removing first
    ///
    /// |        =>      |
    /// \_1              \_2
    /// \_2              \_3
    /// \_3
    ///
    void addThreeRemoveFirst()
    {
        using namespace testing;

        // QVERIFY(true); // check that a condition is satisfied
        ContentFilter cf;
        std::shared_ptr<QQuickView> view(new QQuickView);
        ViewHandler vh(&cf, view);

        /// This is normally called in loadTabs():
        vh.flatModel.setSourceModel(&vh.tabsModel);

        fillWithThree(vh);

        /// getProperty will be called and must return currently selected tab
        EXPECT_CALL(vh.configDb, getProperty(std::string("currentTab")))
            .Times(1).WillOnce(Return(std::string("2")));

        vh.closeTab(1);

        /// Check all models have correct row count
        QCOMPARE(vh.tabsModel.rowCount(), 2);
        QCOMPARE(vh.flatModel.rowCount(), 2);
        QCOMPARE(vh.views2.size(), 2);

        /// Check that items have correct ids, and their order was not changed
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(0))->getId(), 2);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(1))->getId(), 3);

        EXPECT_THROW(vh.flatModel.getModelId(1), std::out_of_range);
        QCOMPARE(vh.flatModel.getModelId(2), 0);
        QCOMPARE(vh.flatModel.getModelId(3), 1);

        QCOMPARE(vh.views2.count(1), 0);
        QCOMPARE(vh.views2.count(2), 1);
        QCOMPARE(vh.views2.count(3), 1);

        QCOMPARE(vh.views2.at(2).tabData->getId(), 2);
        QCOMPARE(vh.views2.at(3).tabData->getId(), 3);

        /// Check internals of flatModel
        QCOMPARE(vh.flatModel.toSource.size(), 2);
        QCOMPARE(vh.flatModel.fromSource.size(), 2);
        QCOMPARE(vh.flatModel.viewId2ModelId.size(), 2);

        QCOMPARE(vh.flatModel.toSource[0]->data().toInt(), 2);
        QCOMPARE(vh.flatModel.toSource[1]->data().toInt(), 3);

        for (unsigned int i = 0; i < vh.flatModel.toSource.size(); ++i)
        {
            auto& item = vh.flatModel.toSource[i];
            QCOMPARE(vh.flatModel.fromSource[item], i);
        }

        QCOMPARE(vh.flatModel.viewId2ModelId[2], 0);
        QCOMPARE(vh.flatModel.viewId2ModelId[3], 1);

    }

    /// Test adding tabs, then removing second
    ///
    /// |        =>      |
    /// \_1              \_1
    /// \_2              \_3
    /// \_3
    ///
    void addThreeRemoveSecond()
    {
        using namespace testing;

        ContentFilter cf;
        std::shared_ptr<QQuickView> view(new QQuickView);
        ViewHandler vh(&cf, view);

        /// This is normally called in loadTabs():
        vh.flatModel.setSourceModel(&vh.tabsModel);

        fillWithThree(vh);

        /// Expected call will set first view as current (after closing second)
        QVariant v = dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()->
                                        child(0))->getView();
        EXPECT_CALL(*vh.webViewContainer,
                    setProperty(std::string("currentView"), v));

        /// And will select first tab as current
        EXPECT_CALL(vh.configDb,
                    setProperty(std::string("currentTab"),
                                std::string("1")));

        /// getProperty will be called and must return currently selected tab
        EXPECT_CALL(vh.configDb, getProperty(std::string("currentTab")))
            .Times(1).WillOnce(Return(std::string("2")));

        vh.closeTab(2);

        /// Check all models have correct row count
        QCOMPARE(vh.tabsModel.rowCount(), 2);
        QCOMPARE(vh.flatModel.rowCount(), 2);
        QCOMPARE(vh.views2.size(), 2);

        /// Check that items have correct ids, and their order was not changed
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(0))->getId(), 1);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(1))->getId(), 3);

        QCOMPARE(vh.flatModel.getModelId(1), 0);
        EXPECT_THROW(vh.flatModel.getModelId(2), std::out_of_range);
        QCOMPARE(vh.flatModel.getModelId(3), 1);

        QCOMPARE(vh.views2.count(1), 1);
        QCOMPARE(vh.views2.count(2), 0);
        QCOMPARE(vh.views2.count(3), 1);

        QCOMPARE(vh.views2.at(1).tabData->getId(), 1);
        QCOMPARE(vh.views2.at(3).tabData->getId(), 3);

        /// Check internals of flatModel
        QCOMPARE(vh.flatModel.toSource.size(), 2);
        QCOMPARE(vh.flatModel.fromSource.size(), 2);
        QCOMPARE(vh.flatModel.viewId2ModelId.size(), 2);

        QCOMPARE(vh.flatModel.toSource[0]->data().toInt(), 1);
        QCOMPARE(vh.flatModel.toSource[1]->data().toInt(), 3);

        for (unsigned int i = 0; i < vh.flatModel.toSource.size(); ++i)
        {
            auto& item = vh.flatModel.toSource[i];
            QCOMPARE(vh.flatModel.fromSource[item], i);
        }

        QCOMPARE(vh.flatModel.viewId2ModelId[1], 0);
        QCOMPARE(vh.flatModel.viewId2ModelId[3], 1);

    }

    /// Test adding tabs, then removing third
    ///
    /// |        =>      |
    /// \_1              \_1
    /// \_2              \_2
    /// \_3
    ///
    void addThreeRemoveThird()
    {
        using namespace testing;

        ContentFilter cf;
        std::shared_ptr<QQuickView> view(new QQuickView);
        ViewHandler vh(&cf, view);

        /// This is normally called in loadTabs():
        vh.flatModel.setSourceModel(&vh.tabsModel);

        fillWithThree(vh);

        /// getProperty will be called and must return currently selected tab
        EXPECT_CALL(vh.configDb, getProperty(std::string("currentTab")))
            .Times(1).WillOnce(Return(std::string("2")));

        vh.closeTab(3);

        /// Check all models have correct row count
        QCOMPARE(vh.tabsModel.rowCount(), 2);
        QCOMPARE(vh.flatModel.rowCount(), 2);
        QCOMPARE(vh.views2.size(), 2);

        /// Check that items have correct ids, and their order was not changed
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(0))->getId(), 1);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(1))->getId(), 2);

        QCOMPARE(vh.flatModel.getModelId(1), 0);
        QCOMPARE(vh.flatModel.getModelId(2), 1);
        EXPECT_THROW(vh.flatModel.getModelId(3), std::out_of_range);

        QCOMPARE(vh.views2.count(1), 1);
        QCOMPARE(vh.views2.count(2), 1);
        QCOMPARE(vh.views2.count(3), 0);

        QCOMPARE(vh.views2.at(1).tabData->getId(), 1);
        QCOMPARE(vh.views2.at(2).tabData->getId(), 2);

        /// Check internals of flatModel
        QCOMPARE(vh.flatModel.toSource.size(), 2);
        QCOMPARE(vh.flatModel.fromSource.size(), 2);
        QCOMPARE(vh.flatModel.viewId2ModelId.size(), 2);

        QCOMPARE(vh.flatModel.toSource[0]->data().toInt(), 1);
        QCOMPARE(vh.flatModel.toSource[1]->data().toInt(), 2);

        for (unsigned int i = 0; i < vh.flatModel.toSource.size(); ++i)
        {
            auto& item = vh.flatModel.toSource[i];
            QCOMPARE(vh.flatModel.fromSource[item], i);
        }

        QCOMPARE(vh.flatModel.viewId2ModelId[1], 0);
        QCOMPARE(vh.flatModel.viewId2ModelId[2], 1);

    }

    /// Test adding 2-level hierarchy of tabs, then removing one parent
    ///
    /// |        =>      |
    /// \_1              \_4
    ///  \_4             \_5
    ///  \_5             \_6
    ///  \_6             \_2
    /// \_2              \_3
    /// \_3
    ///
    void removeParentWithChildrenFirst()
    {
        using namespace testing;

        ContentFilter cf;
        std::shared_ptr<QQuickView> view(new QQuickView);
        ViewHandler vh(&cf, view);

        /// This is normally called in loadTabs():
        vh.flatModel.setSourceModel(&vh.tabsModel);

        fillWithThreePlusChildren(vh, 1);

        /// getProperty will be called and must return currently selected tab
        EXPECT_CALL(vh.configDb, getProperty(std::string("currentTab")))
            .Times(1).WillOnce(Return(std::string("1")));

        /// Expected call will set fourth (new first) view as current
        /// (after closing first)
        QVariant v = dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()->
                                        child(0)->child(0))->getView();
        EXPECT_CALL(*vh.webViewContainer,
                    setProperty(std::string("currentView"), v));


        /// And will select fourth (new first) tab as current
        EXPECT_CALL(vh.configDb,
                    setProperty(std::string("currentTab"),
                                std::string("4")));

        std::cout << "------------------------------------------\n";
        vh.closeTab(1);

        /// Check all models have correct row count
        QCOMPARE(vh.tabsModel.rowCount(), 5);
        for (int i = 0; i < vh.tabsModel.rowCount(); ++i)
        {
            QCOMPARE(vh.tabsModel.invisibleRootItem()->child(i)->rowCount(), 0);
        }
        QCOMPARE(vh.flatModel.rowCount(), 5);
        QCOMPARE(vh.flatModel.toSource.size(), 5);
        QCOMPARE(vh.flatModel.fromSource.size(), 5);
        QCOMPARE(vh.flatModel.viewId2ModelId.size(), 5);
        QCOMPARE(vh.views2.size(), 5);

        /// Check all rows are on correct positions
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(0))->getId(), 4);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(1))->getId(), 5);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(2))->getId(), 6);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(3))->getId(), 2);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(4))->getId(), 3);

        EXPECT_THROW(vh.flatModel.getModelId(1), std::out_of_range);
        QCOMPARE(vh.flatModel.getModelId(2), 3);
        QCOMPARE(vh.flatModel.getModelId(3), 4);
        QCOMPARE(vh.flatModel.getModelId(4), 0);
        QCOMPARE(vh.flatModel.getModelId(5), 1);
        QCOMPARE(vh.flatModel.getModelId(6), 2);

    }

    /// Test adding 2-level hierarchy of tabs, then removing one parent
    ///
    /// |        =>      |
    /// \_1              \_1
    /// \_2              \_4
    ///  \_4             \_5
    ///  \_5             \_6
    ///  \_6             \_3
    /// \_3
    ///
    void removeParentWithChildrenSecond()
    {
        using namespace testing;

        ContentFilter cf;
        std::shared_ptr<QQuickView> view(new QQuickView);
        ViewHandler vh(&cf, view);

        /// This is normally called in loadTabs():
        vh.flatModel.setSourceModel(&vh.tabsModel);

        fillWithThreePlusChildren(vh, 2);

        /// getProperty will be called and must return currently selected tab
        EXPECT_CALL(vh.configDb, getProperty(std::string("currentTab")))
            .Times(1).WillOnce(Return(std::string("1")));

        std::cout << "------------------------------------------\n";
        vh.closeTab(2);

        /// Check all models have correct row count
        QCOMPARE(vh.tabsModel.rowCount(), 5);
        for (int i = 0; i < vh.tabsModel.rowCount(); ++i)
        {
            QCOMPARE(vh.tabsModel.invisibleRootItem()->child(i)->rowCount(), 0);
        }
        QCOMPARE(vh.flatModel.rowCount(), 5);
        QCOMPARE(vh.flatModel.toSource.size(), 5);
        QCOMPARE(vh.flatModel.fromSource.size(), 5);
        QCOMPARE(vh.flatModel.viewId2ModelId.size(), 5);
        QCOMPARE(vh.views2.size(), 5);

        /// Check all rows are on correct positions
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(0))->getId(), 1);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(1))->getId(), 4);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(2))->getId(), 5);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(3))->getId(), 6);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(4))->getId(), 3);

        QCOMPARE(vh.flatModel.getModelId(1), 0);
        EXPECT_THROW(vh.flatModel.getModelId(2), std::out_of_range);
        QCOMPARE(vh.flatModel.getModelId(3), 4);
        QCOMPARE(vh.flatModel.getModelId(4), 1);
        QCOMPARE(vh.flatModel.getModelId(5), 2);
        QCOMPARE(vh.flatModel.getModelId(6), 3);

    }

    /// Test adding 2-level hierarchy of tabs, then removing one parent
    ///
    /// |        =>      |
    /// \_1              \_1
    /// \_2              \_2
    /// \_3              \_4
    ///  \_4             \_5
    ///  \_5             \_6
    ///  \_6
    ///
    void removeParentWithChildrenThird()
    {
        using namespace testing;

        ContentFilter cf;
        std::shared_ptr<QQuickView> view(new QQuickView);
        ViewHandler vh(&cf, view);

        /// This is normally called in loadTabs():
        vh.flatModel.setSourceModel(&vh.tabsModel);

        fillWithThreePlusChildren(vh, 3);

                using namespace testing;

        /// getProperty will be called and must return currently selected tab
        EXPECT_CALL(vh.configDb, getProperty(std::string("currentTab")))
            .Times(1).WillOnce(Return(std::string("1")));

        std::cout << "------------------------------------------\n";
        vh.closeTab(3);

        /// Check all models have correct row count
        QCOMPARE(vh.tabsModel.rowCount(), 5);
        for (int i = 0; i < vh.tabsModel.rowCount(); ++i)
        {
            QCOMPARE(vh.tabsModel.invisibleRootItem()->child(i)->rowCount(), 0);
        }
        QCOMPARE(vh.flatModel.rowCount(), 5);
        QCOMPARE(vh.flatModel.toSource.size(), 5);
        QCOMPARE(vh.flatModel.fromSource.size(), 5);
        QCOMPARE(vh.flatModel.viewId2ModelId.size(), 5);
        QCOMPARE(vh.views2.size(), 5);

        /// Check all rows are on correct positions
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(0))->getId(), 1);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(1))->getId(), 2);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(2))->getId(), 4);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(3))->getId(), 5);
        QCOMPARE(dynamic_cast<Tab*>(vh.tabsModel.invisibleRootItem()
                                    ->child(4))->getId(), 6);

        QCOMPARE(vh.flatModel.getModelId(1), 0);
        QCOMPARE(vh.flatModel.getModelId(2), 1);
        EXPECT_THROW(vh.flatModel.getModelId(3), std::out_of_range);
        QCOMPARE(vh.flatModel.getModelId(4), 2);
        QCOMPARE(vh.flatModel.getModelId(5), 3);
        QCOMPARE(vh.flatModel.getModelId(6), 4);
    }

    /// Removing element from complex structure
    ///
    /// |        =>      |
    /// \_1
    ///  \_4
    ///   \_7
    ///   \_16
    ///   \_17
    ///  \_10
    ///   \_18
    ///   \_19
    ///   \_20
    ///  \_11
    ///   \_21
    ///   \_22
    ///   \_23
    /// \_2
    ///  \_5
    ///   \_8
    ///   \_24
    ///   \_25
    ///  \_12            | x
    ///   \_26
    ///   \_27
    ///   \_28
    ///  \_13
    ///   \_29
    ///   \_30
    ///   \_31
    /// \_3
    ///  \_6
    ///   \_9
    ///  \_14
    ///  \_15

    void complexRemoveMiddle()
    {
        using namespace testing;

        ContentFilter cf;
        std::shared_ptr<QQuickView> view(new QQuickView);
        ViewHandler vh(&cf, view);

        /// This is normally called in loadTabs():
        vh.flatModel.setSourceModel(&vh.tabsModel);

        fillComplexStructure(vh);

        /// getProperty will be called and must return currently selected tab
        EXPECT_CALL(vh.configDb, getProperty(std::string("currentTab")))
            .Times(1).WillOnce(Return(std::string("1")));

        vh.closeTab(12);


        /// FIXME: currently need to supply element that is before the deleted one (on flatModel)
        checkComplexStructure(vh, 25);
    }


    void cleanupTestCase()
    {
//        qDebug("Called after myFirstTest and mySecondTest.");
    }
};


#endif /* VIEWHANDLER_TEST_H_ */
