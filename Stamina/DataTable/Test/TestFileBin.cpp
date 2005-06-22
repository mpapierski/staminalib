#include <stdafx.h>
#include <cppunit/extensions/HelperMacros.h>
#include "..\DataTable.h"
#include "..\FileBin.h"
#include <Stamina\MD5.h>
#include <Stamina\Helpers.h>

using namespace Stamina;
using namespace Stamina::DT;

namespace Stamina { namespace DT {

class TestFileBin : public CPPUNIT_NS::TestFixture
{
  
	CPPUNIT_TEST_SUITE( TestFileBin );
  
	CPPUNIT_TEST( testSave );
	CPPUNIT_TEST( testLoad );

    CPPUNIT_TEST_SUITE_END();

protected:

	ColumnsDesc _cols;
	static const tColId colInt = 1;
	static const tColId colString = 2;
	static const tColId colIntDef = 3;
	static const tColId colStringDef = 4;
	static const tColId col64 = 5;
	static const tColId col64Def = 6;
	static const tColId colBin = 7;
	static const tColId colBinDef = 8;
  
	tRowId row1;
	tRowId row2;
	tRowId row3;

	std::string testString1, testStringDef;
	std::string testString2;
	std::string testString3;
	std::string password;

	int testInt, testIntDef;
	__int64 test64, test64Def;
	TypeBin testBin, testBinDef;

public:
	void setUp() {

		row1 = DataTable::flagId(1);
		row2 = DataTable::flagId(2);
		row3 = DataTable::flagId(3);

		testInt = 100;
		testIntDef = 1000;
		test64 = 0xFF00000000;
		test64Def = 0xAA00000001;
		testBin.size = 10;
		testBin.buff = "1234567890";
		testBinDef.size = 5;
		testBinDef.buff = "12345";
		testString1 = "sadhgasd gadshagd asdgsh dgsajhag dshgd";
		testString2 = "4u535873658653875637856345876345g dshgd";
		testString3 = "AAHSGAJSGASJGASJGJSGASGJSGASHGSHAGSHGSAG";
		testStringDef = "DEFAULT";

		_cols.setColumn(colInt, ctypeInt , 0, "Int");
		_cols.setColumn(colIntDef, ctypeInt | cflagXor, (DataEntry) testIntDef, "IntDef");
		_cols.setColumn(100, ctypeInt | cflagDontSave);
		_cols.setColumn(colString, ctypeString, 0, "String");
		_cols.setColumn(colStringDef, ctypeString | cflagXor, (void*)testStringDef.c_str(), "StringDef");
		_cols.setColumn(101, ctypeString | cflagDontSave);
		_cols.setColumn(col64, ctype64, 0, "64");
		_cols.setColumn(col64Def, ctype64 | cflagXor, &test64Def, "64Def");
		_cols.setColumn(102, ctype64 | cflagDontSave);
		_cols.setColumn(colBin, ctypeBin, 0, "Bin");
		_cols.setColumn(colBinDef, ctypeBin | cflagXor, &testBinDef, "BinDef");
		_cols.setColumn(103, ctypeBin | cflagDontSave);
	}
	void tearDown() {
	}

protected:

	std::string getFileName(const std::string& name) {
		return "TestFileBin__" + name + "_p-" + password + ".dtb";
	}

	void createFile(const std::string& name) {
		DataTable dt;
		dt.mergeColumns(_cols);

		dt.setPassword(password);

		dt.addRow(row1);
		dt.addRow(row2);
		dt.addRow(row3);

		dt.setInt(row1, colInt, testInt);
		dt.set64(row1, col64, test64);
		dt.setBin(row1, colBin, testBin);
		dt.setStr(row1, colString, testString1);
		dt.setStr(row1, colStringDef, testString2);
		dt.setStr(row2, colString, testString2);
		dt.setStr(row3, colString, testString3);

		FileBin fb;
		fb.assign(dt);
		fb.save(getFileName(name));
	}


	void testSave() {
		DataTable dt;
		dt.mergeColumns(_cols);

		dt.setPassword(password);
		dt.addRow(row1);
		dt.addRow(row2);

		FileBin fb;
		fb.assign(dt);


		fb.setColumns( dt.getColumns() );
		CPPUNIT_ASSERT( fb.getColumns().getColCount() > 0 );

		fb.open(getFileName("testSave") , fileWrite);
		CPPUNIT_ASSERT( fb.isOpened() );
		CPPUNIT_ASSERT( fb.isCreatingNewFile() );
		CPPUNIT_ASSERT( fb.isReadable() == false );
		CPPUNIT_ASSERT( fb.isWriteFailed() == false );

		fb.writeHeader();
		CPPUNIT_ASSERT( fb._xorDigest.empty() == false );
		CPPUNIT_ASSERT( fb._xorSalt != 0 );
		CPPUNIT_ASSERT( fb._passwordDigest.empty() == false );
		CPPUNIT_ASSERT( fb._passwordSalt != 0 );
		CPPUNIT_ASSERT( fb.isWriteFailed() == false );
		fb.writeDescriptor();
		CPPUNIT_ASSERT( fb.isWriteFailed() == false );

		fb.writeRow(row1);
		fb.writeRow(row2);

		fb.close();
		CPPUNIT_ASSERT( fb.isWriteFailed() == false );
	}


	void testLoad() {
		createFile("testLoad");

		DataTable dt;
		tColId colString2 = 100;
		dt.setColumn(colString2, ctypeString | cflagXor, (DataEntry) testStringDef.c_str());

		dt.mergeColumns(_cols);

		dt.setPassword(password);

		FileBin fbl;
		fbl.assign(dt);
		CPPUNIT_ASSERT( fbl.load(getFileName("testLoad")) == success );

		CPPUNIT_ASSERT_EQUAL( testString1, dt.getStr(row1, colString) );
		CPPUNIT_ASSERT_EQUAL( testString2, dt.getStr(row1, colStringDef) );
		CPPUNIT_ASSERT_EQUAL( testStringDef, dt.getStr(row2, colStringDef) );
		CPPUNIT_ASSERT_EQUAL( testStringDef, dt.getStr(row2, colString2) );

		dt.setStr(row1, colString, testString2);
		dt.setStr(row2, colString2, testString2);

		FileBin fbs;
		fbs.assign(dt);
		CPPUNIT_ASSERT( fbs.save(getFileName("testLoad")) == success );

		DataTable dt2;
		dt2.setPassword(password);

		FileBin fbl2;
		fbl2.assign(dt2);
		CPPUNIT_ASSERT( fbl2.loadAll(getFileName("testLoad")) == success );

		CPPUNIT_ASSERT_EQUAL( testString2, dt2.getStr(row1, colString) );
		CPPUNIT_ASSERT_EQUAL( testString2, dt2.getStr(row1, colStringDef) );
		CPPUNIT_ASSERT_EQUAL( testStringDef, dt2.getStr(row2, colStringDef) );
		CPPUNIT_ASSERT_EQUAL( testString2, dt2.getStr(row2, colString2) );

	}


};

} }

CPPUNIT_TEST_SUITE_REGISTRATION( TestFileBin );

