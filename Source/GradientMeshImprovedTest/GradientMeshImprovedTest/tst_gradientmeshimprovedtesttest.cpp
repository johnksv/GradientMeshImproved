#include <QString>
#include <QtTest>

class GradientMeshImprovedTestTest : public QObject
{
    Q_OBJECT

public:
    GradientMeshImprovedTestTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testCase1();
};

GradientMeshImprovedTestTest::GradientMeshImprovedTestTest()
{
}

void GradientMeshImprovedTestTest::initTestCase()
{
}

void GradientMeshImprovedTestTest::cleanupTestCase()
{
}

void GradientMeshImprovedTestTest::testCase1()
{
    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(GradientMeshImprovedTestTest)

#include "tst_gradientmeshimprovedtesttest.moc"
