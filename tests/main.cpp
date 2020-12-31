#include "QDX/ClassFactory"

#include <QtTest/QtTest>

class X;

class ClassFactoryTest : public QObject
{
	Q_OBJECT
private slots:
	void initTestCase();

	void testA();
	void testB();

	void cleanupTestCase();

private:
	QDX::ClassFactory<X> m_factory;
};

class X
{
public:
	inline virtual ~X() { }

	virtual int test() const = 0;
};

class A : public X
{
public:
	inline virtual int test() const
	{
		return 111;
	};
};

class B : public X
{
public:
	inline virtual int test() const
	{
		return 222;
	};
};

void ClassFactoryTest::initTestCase()
{
	m_factory.setRegistration(true);
	m_factory.addClass<A>("a");
	m_factory.addClass<B>("b");
}

void ClassFactoryTest::testA()
{
	QVERIFY(m_factory.contains("a"));
	X *a = m_factory.instance("a");
	QCOMPARE(a->test(), 111);
	QCOMPARE(m_factory.record("a")->instances().count(), 1);
	m_factory.destroy("a", a);
	QCOMPARE(m_factory.record("a")->instances().count(), 0);
}

void ClassFactoryTest::testB()
{
	QVERIFY(m_factory.contains("b"));
	X *b = m_factory.instance("b");
	QCOMPARE(b->test(), 222);
	QCOMPARE(m_factory.record("b")->instances().count(), 1);
	m_factory.destroy(b);
	QCOMPARE(m_factory.record("b")->instances().count(), 0);
}

void ClassFactoryTest::cleanupTestCase()
{
	m_factory.removeRecord("a");
	QVERIFY(m_factory.contains("a") == false);
	QCOMPARE(m_factory.records().count(), 1);
	m_factory.removeRecord("b");
	QVERIFY(m_factory.contains("b") == false);
	QCOMPARE(m_factory.records().count(), 0);
}

QTEST_MAIN(ClassFactoryTest)
#include "main.moc"