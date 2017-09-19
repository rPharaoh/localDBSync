#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include <QDateTime>
#include <QMessageBox>
#include <QDir>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void remoteDatabaseConnection();
    void localDatabaseConnection();
    void configDatabaseConnection();

    // configuration data
    void insertSyncTime(int time);
    void setSyncTime(int time);
    void getSyncTime();

    // get local configuration data
    void insertLocalData();
    void setLocalData();
    void getLocalData();

    // get remote configuration data
    void insertRemoteData();
    void setRemoteData();
    void getRemoteData();

    // conference and session data
    void insertConferenceData();
    void setConferenceData();
    void getConferenceData();

    // sync data
    void syncINData();
    void syncOUTData();
    void syncOUTData(int record_id);
    int refrenceToMemberID(int ref_number);
    void updateSyncINDataID(int record_id);
    void updateSyncOUTDataID(int record_id);

    void recordEvents();

    void startTimer();
    void stopTimer();

    // debuging code for the database
    void insertDebugData();
    void removeDebugData();

    // print error to screen
    void print_screen(QString args);

    // plugin loader
    void pluginsLoader();

private slots:
    void syncData();
    void on_update_sync_time_clicked();

    void on_update_local_clicked();

    void on_update_remote_clicked();

    void on_actionReconnect_to_servers_triggered();

    void on_actionInsert_Test_Data_triggered();

    void on_actionRemove_Test_Data_triggered();

    void on_stop_sync_clicked();

    void on_connect_local_db_clicked();

    void on_connect_remote_db_clicked();

    void on_clear_logs_bt_clicked();

    void on_actionExit_triggered();

    void on_start_sync_clicked();

    void on_remote_config_bt_clicked();

    void on_local_config_bt_clicked();

    void on_update_conference_session_bt_clicked();

private:
    Ui::MainWindow *ui;

    // database string
    QSqlDatabase remote_db;
    QSqlDatabase local_db;
    QSqlDatabase config_db;

    // sql query
    QSqlQuery remote_query;
    QSqlQuery local_query;
    QSqlQuery config_query;
    QSqlQuery update_query;
    QSqlQuery sync_out_query;
    QSqlQuery check_record;


    // synic time
    int sync_time;
    int time_timer;

    // debug var, debug mode
    const int debug_number = 10101;

    // datetime for database testing
    QDateTime dateTime;

    // config path
    QString outputDir;
    QString DB_debugPath;
    QString DB_realPath;

    // time for start sync
    QTimer *timer;


    // local server data
    QString server_name_local;
    QString database_name_local;
    QString username_local;
    QString password_local;

    // remote server data
    QString server_name_remote;
    QString database_name_remote;
    QString username_remote;
    QString password_remote;

    // some static data
    QString conference_name;
    int conference_id;

    QString session_name;
    int session_id;

    // animations
    QPropertyAnimation *m_transition_local;
    QPropertyAnimation *m_transition_remote;


};

#endif // MAINWINDOW_H
