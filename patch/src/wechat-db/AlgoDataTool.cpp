/**
* MIT License
*
* Copyright (c) 2015 - 2021
* <https://github.com/bbqz007, http://www.cnblogs.com/bbqzsl>
*/
#include <Zippy/Zippy.ipp>
#include <QtWidgets>
#include <QtCore>
#include <AlgoQ.h>
#include <zqlite3.h>
#include <zqt_helper.h>
#include <QtExtWidgets>

#include <fstream>
#include <memory>
#include <unordered_map>
#include <map>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include "vector_view.hpp"

using namespace zhelper::qt5Widgets;
using namespace zhelper::zqlite3;
#define tr(str) QObject::tr(str)

int load_file(const char* file, std::vector<char>& mem)
{
	std::shared_ptr<std::ifstream> shrd_if(new std::ifstream);
    std::ifstream& in = *shrd_if.get();
    in.open(file, std::ios::binary|std::ios::in);
    in.seekg(0, std::ios::end);
    mem.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&mem[0], mem.size());
    in.close();
	return 0;
}
 
class MyMainFrame : public QFrame
{
public:
	MyMainFrame(QMainWindow* parent = NULL)
		: QFrame(parent) {}
private:
	friend class AlgoQ;
};

#pragma mark - ^_^ wechat hexkey
#define hexkey "paste your hex key here"
template<class Tzqlite3_table>
bool checkif_sqlcipher(Tzqlite3_table& ztbl)
{
	auto cmd = "cipher_version";
	auto iz = ztbl.pragma(cmd);
	iz >> std::ios::beg;
	return !iz.eof();
}
template<class Tzqlite3_table>
void WeChat_dbkey(Tzqlite3_table& ztbl)
{
	if (!checkif_sqlcipher(ztbl))
		QMessageBox::critical(NULL, "sqlite3.dll error", "reopen KTL.exe, a non-cipher version sqlite3.dll has been loaded.");
	
	std::vector<std::string> cmds = {
		"hexkey=\""hexkey"\"", 
		"cipher_hmac_algorithm=HMAC_SHA1",
		"cipher_kdf_algorithm=PBKDF2_HMAC_SHA1",
		"cipher_compatibility = 3",
		"cipher_page_size=4096"};
	for (auto& cmd : cmds)
	{
		ztbl.pragma(cmd) >> std::ios::beg;
	}
}

short byte2hex(unsigned char byte)
{
	const char* table = "0123456789ABCDEF";
	return (table[byte&0xf] <<8) | table[byte>>4];
}

int AlgoQ::main(QMainWindow* win)
{
	//TdxK(0, 0);
	try  
	{  
		auto& C = CLOSE(); 
		auto AMO = AMOUNT();

		auto hisdialog = new QDialog(win);
		auto pickerdlg = new QDialog(win);
		auto readerdlg = new ZQ<QDialog>(win);
		auto p = new MyMainFrame(win);
		auto mainlayout = new QVBoxLayout(p);

		/* -----===== layout the menu =====-------- */
		
		/* -----===== lambda funtions for delegation =====-------- */
		auto parseZip = [=](QTreeWidget* w, const char* path) {
			/* -----===== ZIP =====-------- */
			try {
				Zippy::ZipArchive arch;
				arch.Open(path);
				if (!arch.IsOpen())
					return;
				auto list = arch.GetEntryNames();
				int total = list.size();
				
				auto root = new QTreeWidgetItem (w);
				root->setText(0, "ZIP");
				root->setText(2, QString::fromLocal8Bit(path).toUtf8());
				
				for (int i = 0; i < total; ++i)
				{
					auto& name = list[i];
					auto ent = arch.GetEntry(name);
					auto row = new QTreeWidgetItem(root);
					row->setText(0, name.c_str());
					if (!ent.IsDirectory())
					{
						row->setText(1, "File");
						row->setText(2, std::to_string(ent.UncompressedSize()).c_str());
					}
				}
			}
			catch(std::exception& e)
			{
				QMessageBox::critical(NULL, "ParserZip", e.what());
			}
		}; 
		
		
		auto parseSqlite3 = [=](QTreeWidget* w, const char* path, QTreeWidgetItem* root = NULL) {
			using namespace zhelper::zqlite3;
			/* -----===== SQLITE3 =====-------- */
			auto ztbl = make_zqlite3_table(
				select_para<std::string>("schema"),
				select_para<std::string>("name"),
				select_para<std::string>("type"),
				select_para<int>("ncol"),
				select_para<int>("wr"),
				select_para<int>("strict"));
			ztbl.open_db(QString::fromLocal8Bit(path).toUtf8().data());
			if (!ztbl.quick_check())
			{
				WeChat_dbkey(ztbl);
				
				auto iz = ztbl.pragma("quick_check");
				iz >> std::ios::beg;
				if (iz.err_.err_)
				{
					QMessageBox::critical(NULL, "", "It is not a validate SQLite db");
					return;
				}
			}
				
			if (!root)
				root = new QTreeWidgetItem (w);
			while (root->childCount() > 0)
				delete root->takeChild(0);
			root->setText(0, "SQLITE3");
			root->setText(2, QString::fromLocal8Bit(path).toUtf8());
			
			if (sqlite3_libversion_number() >= 3038001)
			{
				auto iz = ztbl.pragma("table_list");
				auto row = ztbl.create_row();
					
				iz >> std::ios::beg;
				auto zschema = make_zqlite3_table(
					select_para<std::string>("sql"));
				auto res = zschema.create_row();
				zschema.open_db(QString::fromLocal8Bit(path).toUtf8().data());
				for (; !iz.eof(); iz >> std::ios::beg)
				{
					iz >> row;
					auto line = new QTreeWidgetItem(root);
					line->setText(0, (std::get<0>(row) + "." + std::get<1>(row)).c_str());
					line->setText(1, std::get<2>(row).c_str());
					auto izs = zschema.select_from2(("sqlite_master WHERE name='" + std::get<1>(row) + "'").c_str());
					izs >> std::ios::beg;
					if (!izs.eof())
					{
						izs >> res;
						line->setText(2, std::get<0>(res).c_str());
					}
				}
			}
			else
			{
				auto zschema = make_zqlite3_table(
					select_para<std::string>("name"),
					select_para<std::string>("sql"));
				auto res = zschema.create_row();
				zschema.attach_db(ztbl.db_.get());
				auto iz = zschema.select_from2("sqlite_master WHERE type='table'");
				iz >> std::ios::beg;
				for (; !iz.eof(); iz >> std::ios::beg)
				{
					iz >> res;
					auto line = new QTreeWidgetItem(root);
					line->setText(0, std::get<0>(res).c_str());
					line->setText(1, "table");
					line->setText(2, std::get<1>(res).c_str());
				}
				zschema.detach_db();
			}
		};
		
		std::map<const std::string, std::function<void(QTreeWidget* w, const char* path)> > disp;
		disp["ZipParser"] = parseZip;
		disp["Sqlite3Parser"] = parseSqlite3;
		
		auto showLog = [=](std::function<void(QTableWidget*)> delegateShow, QString title) {
			using namespace tblwdgthlp;
			QTableWidget* table = win->findChild<QTableWidget*>("history_data");
			table->clearContents();
			table->model()->removeRows(0, table->rowCount());
			table->model()->removeColumns(0, table->columnCount());
			delegateShow(table);
			table->setVisible(true);
			hisdialog->setWindowTitle(title);
			hisdialog->show();
		};
		
		auto acceptDragFile =
			[](QDragEnterEvent* event){
				if (event->mimeData()->hasFormat("text/uri-list")
					&& event->mimeData()->hasUrls()
					&& event->mimeData()->urls().size() == 1)
				event->acceptProposedAction();
			};

		auto acceptDropFile =
			[=](QDropEvent* event){
				auto w = win->findChild<QTreeWidget*>("ParserTree");
				QString path = QDir(QDir::currentPath()).relativeFilePath(event->mimeData()->urls().at(0).toLocalFile());
				pickerdlg->setResult(0);
				auto grp = pickerdlg->findChild<QButtonGroup*>("ParserPickerResult");
				pickerdlg->exec();
				if (1 != pickerdlg->result())
				{
					QMessageBox::information(NULL, "", "canceled");
					return;
				}
				if (grp)
				{
					auto checked = grp->checkedButton();
					auto it = disp.find(checked->objectName().toStdString());
					if (it != disp.cend())
						it->second(w, path.toLocal8Bit().data());
				}
			};
		
		/* -----===== layout the main frame =====-------- */
		layout::begin(mainlayout)
        ("tdx lday loader, sqlite3 reader", layout::begin(new QVBoxLayout)
			(layout::begin(new QHBoxLayout)
				[QMargins(0, 0, 0, 0)]
				(new ZQ<QLineEdit>("Drag And Drop File Here."))
					[onsignal[&ZQEmitter::dragEnterEvent] = acceptDragFile]
					[onsignal[&ZQEmitter::dropEvent] = acceptDropFile]
				(new QPushButton(" Sqlite3 Console "))
					[onclick = [=](QPushButton* btn){
						static int s_cnt = 0;
						if (s_cnt++ == 0)
							QMessageBox::information(NULL, "", QString("right click sh000001.day on zip tree to load data to sqlite3 db.\n"));
						readerdlg->show();
					}]
				(layout::end))
            (new ZQ<QTreeWidget>)
				[id="ParserTree"]
                [onload=[=](QTreeWidget* w) {
					w->setDragDropMode(QAbstractItemView::InternalMove);
                    w->setColumnCount(3);
                    w->setHeaderLabels(QStringList() << "name" << "type" << "value");
					w->setContextMenuPolicy(Qt::CustomContextMenu);
					
					parseZip(w, "samples\\shlday.zip");
					parseZip(w, "samples\\szlday.zip");
					parseSqlite3(w, "stock.db");
                }]
				[onsignal[&ZQEmitter::dragEnterEvent] = acceptDragFile]
				[onsignal[&ZQEmitter::dropEvent] = acceptDropFile]
				[onsignal=&QTreeWidget::customContextMenuRequested, [=](const QPoint &pos){
					/* -----===== onevent, right click =====-------- */
					auto* treeWidget = win->findChild<QTreeWidget*>("ParserTree");
					QTreeWidgetItem *item = treeWidget->itemAt(pos);
					if (!item)
						return;
					if (item->parent() && item->parent()->text(0) == "ZIP")
                    {
						std::string path = item->parent()->text(2).toLocal8Bit().data();
						std::string tabname = item->text(0).toStdString();
						QRegularExpression re(R"(^s[hz]\d+\.day$)");
						if (!re.match(tabname.c_str()).hasMatch())
						{
							QMessageBox::information(NULL, "", QString("not excepted name\n"));
							return;
						}
						auto m = menu::begin(new QMenu)
							("save to sqlite3 db")[onclick= [=](){
								Zippy::ZipArchive arch;
								arch.Open(path);
								if (!arch.IsOpen())
									return;
								auto data = std::move(arch.GetEntry(tabname).GetData());
								TdxK tk(data.data(), data.size());
								auto C = tk.CLOSE();
								auto Date = tk.DATE();		
								auto Amo = tk.AMO();
								auto Vol = tk.VOL();
								auto Open = tk.OPEN();
								auto High = tk.HIGH();
								auto Low = tk.LOW();
								auto ztbl = make_zqlite3_table(
									select_para<int>("DATE"),
									select_para<double>("OPEN"),
									select_para<double>("HIGH"),
									select_para<double>("LOW"),
									select_para<double>("CLOSE"),
									select_para<int>("VOL"),
									index_para("DATE"));
								ztbl.open_db("stock.db");
								ztbl.create_table(tabname.c_str());
								auto oz = ztbl.insert_into(tabname.c_str());	
								auto oz2 = ztbl.update_where(tabname.c_str(), "WHERE `DATE` = ?");
								oz.begin_trans();
								for (auto i = 0; i < C.size(); ++i)
								{
									oz | Date[i] | Open[i] | High[i] | Low[i] | C[i] | Vol[i] | std::ios::end;
									if (oz.ignored())
										oz2 | Date[i] | Open[i] | High[i] | Low[i] | C[i] | Vol[i] 
											| where_para<int>(Date[i])
											| std::ios::end;
								}
								oz.commit_trans();
								ztbl.close_db();
								
								int Type = 2, Code = 0;
								if (tabname[1] == 'h') Type = 0;
								if (tabname[1] == 'z') Type = 1;
								Code = std::stoi(tabname.substr(2));
								auto lday = make_zqlite3_table(
									select_para<int>("TYPE"),
									select_para<int>("CODE"),
									select_para<int>("DATE"),
									select_para<double>("OPEN"),
									select_para<double>("HIGH"),
									select_para<double>("LOW"),
									select_para<double>("CLOSE"),
									select_para<int>("VOL"),
									index_para("TYPE", "CODE", "DATE"));
								lday.open_db("stock.db");
								lday.create_table("lday");
								{
									auto oz = lday.insert_into("lday");	
									auto oz2 = lday.update_where("lday", "WHERE `TYPE` = ? and `CODE` = ? and `DATE` = ?");
									oz.begin_trans();
									for (auto i = 0; i < C.size(); ++i)
									{
										oz | Type| Code | Date[i] | Open[i] | High[i] | Low[i] | C[i] | Vol[i] | std::ios::end;
										if (oz.ignored())
											oz2 | Type| Code | Date[i] | Open[i] | High[i] | Low[i] | C[i] | Vol[i] 
												| where_para<int>(Type)| where_para<int>(Code)| where_para<int>(Date[i])
												| std::ios::end;
									}
									oz.commit_trans();
								}
								lday.close_db();
							}]
							("save ma to sqlite3 db")[onclick= [=](){
								Zippy::ZipArchive arch;
								arch.Open(path);
								if (!arch.IsOpen())
									return;
								auto data = std::move(arch.GetEntry(tabname).GetData());
								TdxK tk(data.data(), data.size());
								auto C = MA(tk.CLOSE(), 5);
								auto Date = REF(tk.DATE(), -4).copy();		
								auto Amo = MA(tk.AMO(), 5);
								auto Vol = MA(tk.VOL(), 5);
								auto Open = MA(tk.OPEN(), 5);
								auto High = MA(tk.HIGH(), 5);
								auto Low = MA(tk.LOW(), 5);
								auto ztbl = make_zqlite3_table(
									select_para<int>("DATE"),
									select_para<double>("OPEN"),
									select_para<double>("HIGH"),
									select_para<double>("LOW"),
									select_para<double>("CLOSE"),
									select_para<int>("VOL"),
									index_para("DATE"));
								ztbl.open_db("stock.db");
								ztbl.create_table((tabname + ".ma5").c_str());
								auto oz = ztbl.insert_into((tabname + ".ma5").c_str());	
								auto oz2 = ztbl.update_where((tabname + ".ma5").c_str(), "WHERE `DATE` = ?");
								oz.begin_trans();
								for (auto i = 0; i < C.size(); ++i)
								{
									oz | Date[i] | Open[i] | High[i] | Low[i] | C[i] | Vol[i] | std::ios::end;
									if (oz.ignored())
										oz2 | Date[i] | Open[i] | High[i] | Low[i] | C[i] | Vol[i] 
											| where_para<int>(Date[i])
											| std::ios::end;
								}
								oz.commit_trans();
								ztbl.close_db();
								
								int Type = 2, Code = 0;
								if (tabname[1] == 'h') Type = 0;
								if (tabname[1] == 'z') Type = 1;
								Code = std::stoi(tabname.substr(2));
								auto lday = make_zqlite3_table(
									select_para<int>("TYPE"),
									select_para<int>("CODE"),
									select_para<int>("DATE"),
									select_para<double>("OPEN"),
									select_para<double>("HIGH"),
									select_para<double>("LOW"),
									select_para<double>("CLOSE"),
									select_para<int>("VOL"),
									index_para("TYPE", "CODE", "DATE"));
								lday.open_db("stock.db");
								lday.create_table("lma5");
								{
									auto oz = lday.insert_into("lma5");	
									auto oz2 = lday.update_where("lma5", "WHERE `TYPE` = ? and `CODE` = ? and `DATE` = ?");
									oz.begin_trans();
									for (auto i = 0; i < C.size(); ++i)
									{
										oz | Type| Code | Date[i] | Open[i] | High[i] | Low[i] | C[i] | Vol[i] | std::ios::end;
										if (oz.ignored())
											oz2 | Type| Code | Date[i] | Open[i] | High[i] | Low[i] | C[i] | Vol[i] 
												| where_para<int>(Type)| where_para<int>(Code)| where_para<int>(Date[i])
												| std::ios::end;
									}
									oz.commit_trans();
								}
								lday.close_db();
							}]
							(menu::end);
						m->exec(treeWidget->mapToGlobal(pos));
					}
					else if (!item->parent() && item->text(0) == "SQLITE3")
					{
						auto m = menu::begin(new QMenu)
							("reload sqlite3 db")[onclick= [=](){
								parseSqlite3(treeWidget, "stock.db", item);
							}]
							(menu::end);
						m->exec(treeWidget->mapToGlobal(pos));
					}
				}] 
				[onsignal=&QTreeWidget::itemDoubleClicked, [=](QTreeWidgetItem* item, int col){
						/* -----===== onevent, double click zip =====-------- */
					if (item->parent() && item->parent()->text(0) == "ZIP")
                    {
						std::string path = item->parent()->text(2).toLocal8Bit().data();
						std::string tabname = item->text(0).toStdString();
						Zippy::ZipArchive arch;
						arch.Open(path);
						if (!arch.IsOpen())
							return;
						QRegularExpression re(R"(^s[hz]\d+\.day$)");
						if (!re.match(tabname.c_str()).hasMatch())
						{
							QMessageBox::information(NULL, "", QString("not excepted name\n"));
							return;
						}
						auto data = std::move(arch.GetEntry(tabname).GetData());
						TdxK tk(data.data(), data.size());
						auto O = tk.OPEN();
						auto H = tk.HIGH();
						auto L = tk.LOW();
						auto C = tk.CLOSE();
						auto Date = tk.DATE();		
						auto Amo = tk.AMO();
						auto Vol = tk.VOL();
						showLog([&](QTableWidget* table) {
							using namespace tblwdgthlp;
							column::begin(table)
								("date")
								("open")
								("high")
								("low")
								("close")
								("vol")
								("amount")
								(column::end);
							auto& cursor = row::begin(table);
							for (int i = 0; i < Date.size(); ++i)
							{
								cursor
								(QString::number(Date[i]))
								(QString::number(O[i]))
								(QString::number(H[i]))
								(QString::number(L[i]))
								(QString::number(C[i]))
								(QString::number((int)Vol[i]))
								(QString::number((int)Amo[i]))
								(row::another);
							}
								cursor(row::end);
						}, tabname.c_str());
					}
					/* -----===== onevent, double click sqlite3 db =====-------- */
					else if (item->parent() && item->parent()->text(0) == "SQLITE3")
                    {
						std::string path = item->parent()->text(2).toLocal8Bit().data();
						std::string tabname = item->text(0).toStdString();
						auto dot = tabname.find(".");
						if (sqlite3_libversion_number() >= 3038001 && dot != -1)
							tabname = tabname.substr(dot+1);
						std::string sql = "select * from `" + tabname + "`";
                        QTableWidget* table = hisdialog->findChild<QTableWidget*>("history_data");
						const char* problem = 0;
						int err = 0, changes = 0;
						Db* db = 0;
						DbStmt* stmt = 0; 
						std::shared_ptr<Db> db_p((err = sqlite3_open(path.c_str(), &db), db), sqlite3_close);
						err = sqlite3_exec(db, "pragma quick_check;", 0, 0, 0);
						if (!db || SQLITE_OK != err)
						{
							using namespace zhelper::zqlite3;
							auto ztbl = make_zqlite3_table(select_para<std::string>("null"));
							ztbl.attach_db(db);
							WeChat_dbkey(ztbl);
							ztbl.detach_db();
							char* pszErr = NULL;
							if (sqlite3_exec(db, "pragma quick_check;", 0, 0, &pszErr))
							{
								QMessageBox::information(NULL, "", pszErr);
								return ;
							}
						}
						std::shared_ptr<DbStmt> dbstmt_p((err = sqlite3_prepare(db, sql.c_str(), sql.size(), &stmt, &problem), stmt), sqlite3_finalize);
						if (SQLITE_OK != err)
						{
							QMessageBox::information(NULL, "failed to prepare", sql.c_str());
							return ;
						}
						changes = sqlite3_total_changes(db);
						err = sqlite3_step(stmt);
						if (SQLITE_OK != err && SQLITE_ROW != err)
						{
							if (SQLITE_DONE == err)
								QMessageBox::information(NULL, "", "(0) records");
							return ;
						}
						if (SQLITE_OK == err)
						{
							changes = sqlite3_total_changes(db) - changes;
							QMessageBox::information(NULL, "", QString("affected row: %1\n").arg(changes));
							return;
						}
						using namespace tblwdgthlp;
						table->clearContents();
						table->model()->removeRows(0, table->rowCount());
						table->model()->removeColumns(0, table->columnCount());
						auto& cols = column::begin(table);
						for (int i = 0, end = sqlite3_column_count(stmt); i < end; ++i)
						{
							cols(sqlite3_column_name(stmt, i));
						}
						cols(column::end);
						auto& rows = row::begin(table);
						std::string value;
						value.reserve(4096);
						const char* blob = NULL;
						for (; err == SQLITE_ROW; err = sqlite3_step(stmt))
						{
							for (int i = 0, end = sqlite3_column_count(stmt); i < end; ++i)
							{
								blob = NULL;
								switch (sqlite3_column_type(stmt, i))
								{
								case SQLITE_INTEGER:
									value = std::to_string(sqlite3_column_int64(stmt, i)); break;
								case SQLITE_FLOAT:
									value = std::to_string(sqlite3_column_double(stmt, i)); break;
								case SQLITE_TEXT:
									value = (char*)sqlite3_column_text(stmt, i); break;
								case SQLITE_BLOB:
									blob = (char*)sqlite3_column_blob(stmt, i); 
									value.resize(sqlite3_column_bytes(stmt, i)*2);
									for (auto i = 0; i < value.size(); i+=2)
									{
										*((short*)&value[i]) = byte2hex(blob[i/2]);
									}
									break;
								default:
									value.resize(0);
									break;
								}
								rows(value.c_str());
							}
							rows(row::another);
						}
						rows(row::end);
						table->setVisible(true);
						hisdialog->setWindowTitle(tabname.c_str());
						hisdialog->show();
                    }
                }]
            (layout::end))
        (layout::end, [=](QLayout*) {
            p->setLayout(mainlayout);
            win->setCentralWidget(p);
            win->setWindowTitle("zhelper Data Analysis");
            win->setMinimumWidth(800);
            win->setMinimumHeight(700);
        });
		
		/* -----===== layout picker dialog Frame =====-------- */
		QRadioButton* rbtn[2];
		layout::begin(new QVBoxLayout)
		("Pick a Parser", layout::begin(new QVBoxLayout)
			[QMargins(0, 0, 0, 0)]
			(rbtn[0] = new QRadioButton("ZipParser"))[id="ZipParser"]
			(rbtn[1] = new QRadioButton("Sqlite3Parser"))[id="Sqlite3Parser"]
			(new QPushButton("OK"))
				[onclick=[=](QPushButton* btn){
					auto grp = pickerdlg->findChild<QButtonGroup*>("ParserPickerResult");
					if (grp && grp->checkedButton())
					{
						pickerdlg->setResult(1);
						pickerdlg->done(1);
					}
				}]
			(layout::end))
			(layout::end,
			 layout::oncomplete =
				[=](QLayout* s) {
					QButtonGroup* grp = new QButtonGroup(pickerdlg);
					grp->setObjectName("ParserPickerResult");
					for (int i = 0; i < sizeof(rbtn)/sizeof(rbtn[0]); ++i)
						grp->addButton(rbtn[i]);
					rbtn[0]->setChecked(true);
					pickerdlg->setLayout(s);
					pickerdlg->setWindowTitle("ParserPicker");
					pickerdlg->setMinimumWidth(200);
					pickerdlg->setMinimumHeight(400);
			 });

		/* -----===== layout sqlite3 result dialog Frame =====-------- */
		layout::begin(new QVBoxLayout)
			[QMargins(0, 0, 0, 0)]
			(new QTableWidget)
				[id="history_data"]
				[onload= [=](QTableWidget* table) {
					table->setVisible(false);
					table->setProperty("sortcol", 0);
					QObject::connect(table->horizontalHeader(), &QHeaderView::sectionClicked, [=](int col){
						int sortcol = table->property("sortcol").toInt();
						if (abs(sortcol) == col + 1)
							sortcol *= -1;
						else
							sortcol = col + 1;
						table->setProperty("sortcol", sortcol);
						table->sortItems(col, (sortcol) > 0 ? Qt::DescendingOrder : Qt::AscendingOrder);
					});
				}]
			(layout::end,
			 layout::oncomplete =
				[=](QLayout* s) {
					hisdialog->setLayout(s);
					hisdialog->setWindowTitle("history");
					hisdialog->setMinimumWidth(800);
					hisdialog->setMinimumHeight(600);
			 });
		
		/* -----===== layout sqlite3 console dialog Frame =====-------- */
		layout::begin(new QVBoxLayout)
			("sqlite3",
			layout::begin(new QHBoxLayout)
				[QMargins(0, 0, 0, 0)]
				(new QLabel("sql"))
				(new QLineEdit("select 'right click sh000001.day on zip tree to load data to sqlite3 db.';"))
					[id="SQL_COMMAND"]
				(new QPushButton("ExecSQL"))
					[onclick=[=](QPushButton*){
						QTableWidget* table = hisdialog->findChild<QTableWidget*>("history_data");
						QLineEdit* sqlinput = win->findChild<QLineEdit*>("SQL_COMMAND");
						QTextEdit* log = win->findChild<QTextEdit*>("output_log");
						std::string sql = sqlinput->text().toStdString() + ";";
						std::string path = win->findChild<QLineEdit*>("path_to_load")->text().toStdString();
						const char* problem = 0;
						int err = 0, changes = 0;
						Db* db = 0;
						DbStmt* stmt = 0;
						std::shared_ptr<Db> db_p((err = sqlite3_open(path.c_str(), &db), db), sqlite3_close);
						err = sqlite3_exec(db, "pragma quick_check;", 0, 0, 0);
						if (!db || SQLITE_OK != err)
						{
							using namespace zhelper::zqlite3;
							auto ztbl = make_zqlite3_table(select_para<std::string>("null"));
							ztbl.attach_db(db);
							WeChat_dbkey(ztbl);
							ztbl.detach_db();
							if (sqlite3_exec(db, "pragma quick_check;", 0, 0, 0))
							{
								log->append(QString("opendb (%1) failed: %2\n").arg(path.c_str(), (db) ? sqlite3_errstr(err) : " is not a valid sqlite3 db"));
								return ;
							}
						}
						std::shared_ptr<DbStmt> dbstmt_p((err = sqlite3_prepare(db, sql.c_str(), sql.size(), &stmt, &problem), stmt), sqlite3_finalize);
						if (SQLITE_OK != err)
						{
							log->append(QString("prepare (%1) failed: %2\n at %3\n").arg(sql.c_str(), sqlite3_errstr(err), problem));
							return ;
						}
						changes = sqlite3_total_changes(db);
						err = sqlite3_step(stmt);
						if (SQLITE_OK != err && SQLITE_ROW != err)
						{
							if (SQLITE_DONE == err)
								log->append(QString("(0) records\n"));
							else
								log->append(QString("step (%1) failed: %2\n").arg(sql.c_str(), sqlite3_errstr(err)));
							return ;
						}
						log->append(QString("ExecSQL (%1)\n").arg(sql.c_str())); 
						if (SQLITE_OK == err)
						{
							changes = sqlite3_total_changes(db) - changes;
							QMessageBox::information(NULL, "", QString("affected row: %1\n").arg(changes));
							return;
						}
						using namespace tblwdgthlp;
						table->clearContents();
						table->model()->removeRows(0, table->rowCount());
						table->model()->removeColumns(0, table->columnCount());
						auto& cols = column::begin(table);
						for (int i = 0, end = sqlite3_column_count(stmt); i < end; ++i)
						{
							cols(sqlite3_column_name(stmt, i));
						}
						cols(column::end);
						auto& rows = row::begin(table);
						std::string value;
						value.reserve(4096);
						const char* blob = NULL;
						for (; err == SQLITE_ROW; err = sqlite3_step(stmt))
						{
							for (int i = 0, end = sqlite3_column_count(stmt); i < end; ++i)
							{
								blob = NULL;
								switch (sqlite3_column_type(stmt, i))
								{
								case SQLITE_INTEGER:
									value = std::to_string(sqlite3_column_int64(stmt, i)); break;
								case SQLITE_FLOAT:
									value = std::to_string(sqlite3_column_double(stmt, i)); break;
								case SQLITE_TEXT:
									value = (char*)sqlite3_column_text(stmt, i); break;
								case SQLITE_BLOB:
									blob = (char*)sqlite3_column_blob(stmt, i); 
									value.resize(sqlite3_column_bytes(stmt, i)*2);
									for (auto i = 0; i < value.size(); i+=2)
									{
										*((short*)&value[i]) = byte2hex(blob[i/2]);
									}
									break;
								default:
									value.resize(0);
									break;
								}
								rows(value.c_str());
							}
							rows(row::another);
						}
						rows(row::end);
						table->setVisible(true);
						hisdialog->setWindowTitle("K Line Table");
						hisdialog->show();
					}]
				(layout::end))
					
			/* -----===== layout other samples =====-------- */
			("Environment: (drag and drop db file)",
			 layout::begin(new QVBoxLayout)
				[QMargins(0, 0, 0, 0)]
				(layout::begin(new QHBoxLayout)
					(new ZQ<QLineEdit>)
						["path_to_load"]
						[onload= [=](QLineEdit* edit){
								edit->setText("stock.db");
								edit->setReadOnly(true);
							}]
						[onsignal[&ZQEmitter::dragEnterEvent] = acceptDragFile]
						[onsignal = &ZQEmitter::dropEvent,
							[=](QDropEvent* event){
								QLineEdit* edit = win->findChild<QLineEdit*>("path_to_load");
								QString path = QDir(QDir::currentPath()).relativeFilePath(event->mimeData()->urls().at(0).toLocalFile());
								edit->setText(path);
							}]
					(new QPushButton)
						["data..."]
						[onclick= 
							[=](QPushButton*){
								QUrl url = QFileDialog::getOpenFileUrl(win,
																	   "path_to_load", QUrl(),
																	   "(*.*)");
								QString newpath = url.path();
								if (!newpath.isEmpty())
									win->findChild<QLineEdit*>("path_to_load")->setText(newpath);
							}]
					(layout::end))
				(layout::end))
			("Output Log: (logger sample)",
			 layout::begin(new QVBoxLayout)
				[QMargins(0, 0, 0, 0)]
				(layout::begin(new QHBoxLayout)
					(new QTextEdit)
						["output_log"]
						[onload= [=](QTextEdit* log) {
							log->setReadOnly(true);
							log->resize(log->width(), 300);
							log->append("welcome to zhelper.(a demo written with zqt_helper)\n");
							log->append("right click sh000001.day on zip tree to load data to sqlite3 db.\n");
						} ]
					(layout::end))

				(layout::end))
			
			
			(layout::end,
			 layout::oncomplete =
				[=](QLayout* s) {
					readerdlg->setLayout(s);
					readerdlg->setWindowTitle("sqlite3reader");
					readerdlg->setMinimumWidth(800);
					readerdlg->setMinimumHeight(600);
					readerdlg->setVisible(false);
			 });
		return 0;
	}
	catch (...)
	{

	}
	return -1;
}
