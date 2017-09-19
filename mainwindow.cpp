#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // plugins loader
    //pluginsLoader();

    // timer defualt
    sync_time = 10;

    // path var
    outputDir = QDir::currentPath(); // with "/" separators

    // config data init
    configDatabaseConnection();
    getSyncTime(); // get sync time
    getLocalData(); // local database string
    getRemoteData(); // remote database string
    getConferenceData(); // get conferece data session and conference

    // local data handler
    localDatabaseConnection();

    // handleing remote connection
    // ===========================
    remoteDatabaseConnection();


    // start timer for sync
    startTimer();

    // create animation class for transition
    m_transition_local = new QPropertyAnimation(ui->local_db_config_2, "pos", this);
    m_transition_remote = new QPropertyAnimation(ui->remote_db_config_2, "pos", this);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::remoteDatabaseConnection()
{
    /*
     *  Remote Connections
     */
    remote_db = QSqlDatabase::addDatabase("QMYSQL", "Remote Database");
    /* for my test pc in home
    remote_db.setHostName("localhost");
    remote_db.setDatabaseName("con");
    remote_db.setUserName("root");
    remote_db.setPassword("");
    */

    // for real work until modification
    // ------------------------------------
    remote_db.setHostName(server_name_remote);
    remote_db.setDatabaseName(database_name_remote);
    remote_db.setUserName(username_remote);
    remote_db.setPassword(password_remote);
    // ------------------------------------

    if(remote_db.open()){
        this->remote_query = QSqlQuery(remote_db);
        this->check_record = QSqlQuery(remote_db);

        print_screen("Successful Connection to remote database.");
    }
    else {
        print_screen("Failed to connect to remote database." + remote_query.lastError().text());
    }

}

void MainWindow::localDatabaseConnection()
{
    /*
     *  Local Connections
     */

    //* for my test pc in home
    //QString server_name = "RFID-Computer";
    //QString database_name = "RFID";
    //QString username = "RFID_User";
    //QString password = "RFID_Password";
    //*/

    // for the local database MS SQL Server
    // ------------------------------------
    QString server_name = server_name_local;
    QString database_name = database_name_local;
    QString username = username_local;
    QString password = password_local;
    // ------------------------------------

    local_db = QSqlDatabase::addDatabase("QODBC", "Local Database");
    local_db.setDatabaseName("DRIVER={SQL Server Native Client 11.0};SERVER=" + server_name + ";DATABASE=" + database_name +
                             ";UID=" + username + ";PWD=" + password + ";WSID=.;Trusted_connection=yes");

    if(local_db.open()){
        this->local_query = QSqlQuery(local_db);
        this->sync_out_query = QSqlQuery(local_db);
        this->update_query = QSqlQuery(local_db);
        print_screen("Successful Connection to local database.");
    }
    else {
        QString err_str = local_db.lastError().text();
        print_screen("Failed to connect to local database." + err_str);
    }
}

void MainWindow::configDatabaseConnection()
{
    DB_debugPath = "C:/Users/rPharaoh/Documents/Projects/Loader/Loader/config.db";
    DB_realPath = "/config.db";

    // Real Path: (outputDir + DB_realPath)
    config_db = QSqlDatabase::addDatabase("QSQLITE", "ConfigurationDB");
    config_db.setDatabaseName(outputDir + DB_realPath);

    if (!config_db.open())
    {
        QMessageBox::critical(0, QObject::tr("Configration Database Error!"),
                              config_db.lastError().text());
        // "Configration Database Error: ", config_db.lastError().text();
    }
    else {
        print_screen("Configration Database opened success");
        this->config_query = QSqlQuery(config_db);
    }
}

void MainWindow::setSyncTime(int time)
{
    config_query.prepare("UPDATE config_data SET sync_time = :sync_time WHERE id = :id");

    config_query.bindValue(":id", 1);
    config_query.bindValue(":sync_time", time);

    if(config_query.exec())
    {
        print_screen("Successful inserting to config database.");
        ui->txt_msg->setText("Sync time has been updated");
    } else {
        print_screen("Error!, inserting to config database faild.");

        if(config_db.open())
        {
            this->insertSyncTime(time);
        }
        else {
            ui->txt_msg->setText("Faild to update sync time.");
        }
    }
}

void MainWindow::insertSyncTime(int time)
{
    config_query.prepare("INSERT INTO config_data (id, sync_time) VALUES (:id, :sync_time) ");

    config_query.bindValue(":id", 1);
    config_query.bindValue(":sync_time", time);

    if(config_query.exec())
    {
        print_screen("Successful inserting to config database.");
    } else {
        print_screen("Error!, inserting to config database faild.");
    }
}


void MainWindow::getSyncTime()
{
    config_query.prepare("SELECT * FROM config_data");
    if(config_query.exec())
    {
        while(config_query.next()) {
            ui->sync_time->setText(config_query.value(1).toString());
            this->sync_time = config_query.value(1).toInt();
        }
    }
}

void MainWindow::insertLocalData()
{
    config_query.prepare("INSERT INTO database_config (username, password, server_name, database_name, config_type) "
                         "VALUES (:username, :password, :server_name, :database_name, :config_type) ");

    config_query.bindValue(":username", username_local);
    config_query.bindValue(":password", password_local);
    config_query.bindValue(":server_name", server_name_local);
    config_query.bindValue(":database_name", database_name_local);
    config_query.bindValue(":config_type", "local_db");

    if(config_query.exec())
    {
        print_screen("Successful inserting to local database configuration.");
    } else {
        print_screen("Error!, inserting to local database configuration faild.");
    }
}

void MainWindow::setLocalData()
{
    config_query.prepare("UPDATE database_config SET username = :username, password = :password, "
                         "server_name = :server_name, database_name = :database_name WHERE config_type = 'local_db' ");

    config_query.bindValue(":username", username_local);
    config_query.bindValue(":password", password_local);
    config_query.bindValue(":server_name", server_name_local);
    config_query.bindValue(":database_name", database_name_local);

    if(config_query.exec())
    {
        print_screen("Successful updated local database configuration.");
        ui->txt_local_msg->setText("local database configuration has been updated");
    } else {
        print_screen("Error!, updating local database configuration faild.");

        if(config_db.open())
        {
            this->insertLocalData();
        }
        else {
            ui->txt_local_msg->setText("Faild to update local database configuration.");
        }
    }

}

void MainWindow::getLocalData()
{
    config_query.prepare("SELECT * FROM database_config WHERE config_type = 'local_db'");
    if(config_query.exec())
    {
        while(config_query.next()) {

            // for the ui
            ui->txt_username_local->setText(config_query.value(1).toString());
            ui->txt_password_local->setText(config_query.value(2).toString());
            ui->txt_server_name_local->setText(config_query.value(3).toString());
            ui->txt_database_name_local->setText(config_query.value(4).toString());

            // save to local variables
            this->username_local = config_query.value(1).toString();
            this->password_local = config_query.value(2).toString();
            this->server_name_local = config_query.value(3).toString();
            this->database_name_local = config_query.value(4).toString();
        }
    }
}

void MainWindow::insertRemoteData()
{
    config_query.prepare("INSERT INTO database_config (username, password, server_name, database_name, config_type) "
                         "VALUES (:username, :password, :server_name, :database_name, :config_type) ");

    config_query.bindValue(":username", username_remote);
    config_query.bindValue(":password", password_remote);
    config_query.bindValue(":server_name", server_name_remote);
    config_query.bindValue(":database_name", database_name_remote);
    config_query.bindValue(":config_type", "remote_db");

    if(config_query.exec())
    {
        print_screen("Successful inserting to remote database configuration.");
    } else {
        print_screen("Error!, inserting to remote database configuration faild.");
    }

}

void MainWindow::setRemoteData()
{
    config_query.prepare("UPDATE database_config SET username = :username, password = :password,"
                         "server_name = :server_name, database_name = :database_name WHERE config_type = 'remote_db' ");

    config_query.bindValue(":username", username_remote);
    config_query.bindValue(":password", password_remote);
    config_query.bindValue(":server_name", server_name_remote);
    config_query.bindValue(":database_name", database_name_remote);

    if(config_query.exec())
    {
        print_screen("Successful updated remote database configuration.");
        ui->txt_remote_msg->setText("remote database configuration has been updated");
    } else {
        print_screen("Error!, updating remote database configuration faild.");

        if(config_db.open())
        {
            this->insertRemoteData();
        }
        else {
            ui->txt_remote_msg->setText("Faild to update remote database configuration.");
        }
    }

}

void MainWindow::getRemoteData()
{
    config_query.prepare("SELECT * FROM database_config WHERE config_type = 'remote_db'");
    if(config_query.exec())
    {
        while(config_query.next()) {

            // save to local variables
            this->username_remote = config_query.value(1).toString();
            this->password_remote = config_query.value(2).toString();
            this->server_name_remote = config_query.value(3).toString();
            this->database_name_remote = config_query.value(4).toString();

            // for the ui
            ui->txt_username_remote->setText(config_query.value(1).toString());
            ui->txt_password_remote->setText(config_query.value(2).toString());
            ui->txt_server_name_remote->setText(config_query.value(3).toString());
            ui->txt_database_name_remote->setText(config_query.value(4).toString());
        }
    }
}

void MainWindow::insertConferenceData()
{
    config_query.prepare("INSERT INTO conference_data (id, conference_name, conference_id, session_name, session_id) "
                         "VALUES (:id, :conference_name, :conference_id, :session_name, :session_id) ");

    config_query.bindValue(":id", 1);
    config_query.bindValue(":conference_name", conference_name);
    config_query.bindValue(":conference_id", conference_id);
    config_query.bindValue(":session_name", session_name);
    config_query.bindValue(":session_id", session_id);

    if(config_query.exec())
    {
        print_screen("Successful inserting to conference database configuration." + config_query.lastError().text());
    } else {
        print_screen("Error!, inserting to conference database configuration faild." + config_query.lastError().text());
    }

}

void MainWindow::setConferenceData()
{
    config_query.prepare("UPDATE conference_data SET conference_name = :conference_name, conference_id = :conference_id,"
                         "session_name = :session_name, session_id = :session_id WHERE id = :id ");

    config_query.bindValue(":id", 1);
    config_query.bindValue(":conference_name", conference_name);
    config_query.bindValue(":conference_id", conference_id);
    config_query.bindValue(":session_name", session_name);
    config_query.bindValue(":session_id", session_id);

    if(config_query.exec())
    {
        print_screen("Successful updated conference database configuration.");
        ui->txt_conference_session_msg->setText("conference database configuration has been updated");
    } else {
        print_screen("Error!, updating conference database configuration faild." + config_query.lastError().text());

        if(config_db.open())
        {
            this->insertConferenceData();
        }
        else {
            ui->txt_conference_session_msg->setText("Faild to update conference database configuration." + config_query.lastError().text());
        }
    }
}

void MainWindow::getConferenceData()
{
    config_query.prepare("SELECT * FROM conference_data WHERE id = :id ");
    config_query.bindValue(":id", 1);

    if(config_query.exec())
    {
        while(config_query.next()) {

            // save to local variables
            this->conference_name = config_query.value(1).toString();
            this->conference_id = config_query.value(2).toInt();
            this->session_name = config_query.value(3).toString();
            this->session_id = config_query.value(4).toInt();

            // for the ui
            ui->txt_conference_name->setText(config_query.value(1).toString());
            ui->txt_conference_id->setText(config_query.value(2).toString());
            ui->txt_session_name->setText(config_query.value(3).toString());
            ui->txt_session_id->setText(config_query.value(4).toString());
        }
    }
}

void MainWindow::syncData()
{
    syncINData();
    syncOUTData();
    startTimer();
}

void MainWindow::syncINData()
{
    QString sql_string = "SELECT * FROM TBL_ATTENDANCE WHERE INT_TIME != 0 AND SYNC_IN IS NULL";
    local_query.prepare(sql_string);

    if(local_query.exec())
    {
        while(local_query.next())
        {

            // --------------------------
            // post data to remote server
            // --------------------------

            remote_query.prepare("INSERT INTO credit_hours "
                                 "(conference_id, session_id, member_id, check_in) "
                                 "VALUES (:conference_id, :session_id, :member_id, :check_in)");

            remote_query.bindValue(":conference_id", conference_id);
            remote_query.bindValue(":session_id", session_id);
            remote_query.bindValue(":member_id", local_query.value(1).toString());
            remote_query.bindValue(":check_in", local_query.value(3).toDateTime());
            //remote_query.bindValue(":check_out", local_query.value(4).toDateTime());

            if(remote_query.exec())
            {
                print_screen("Successfuly synced, record ID: " + local_query.value(0).toString());

                // ---------------------------------
                // change local server SYNC_IN to 1
                // successfuly synced this record
                // ---------------------------------
                updateSyncINDataID(local_query.value(0).toInt());

            } else { print_screen("Could not sync record to remote server " + remote_query.lastError().text()); }
        }
    } else { print_screen("Could not get record from local server"  + remote_query.lastError().text()); }
}

void MainWindow::syncOUTData()
{
    QString sql_string = "SELECT * FROM TBL_ATTENDANCE WHERE OUT_TIME != 0 AND SYNC_OUT IS NULL";
    local_query.prepare(sql_string);

    if(local_query.exec())
    {
        while(local_query.next())
        {
            int member_id = refrenceToMemberID(local_query.value(1).toInt());
            syncOUTData(member_id);
        }
    }
}

void MainWindow::syncOUTData(int member_id)
{
    int counter, record_m_id;

    QString check_id_string = "SELECT id FROM credit_hours c2 WHERE c2.member_id = :member_id AND c2.conference_id =:conference_id AND c2.session_id = :session_id AND c2.check_out = 0";

    check_record.prepare(check_id_string);
    remote_query.bindValue(":member_id", member_id);
    remote_query.bindValue(":conference_id", conference_id);
    remote_query.bindValue(":session_id", session_id);

    if(check_record.exec())
    {
        counter = 0;
        while (check_record.next()) {
            counter++;
            record_m_id = check_record.value(0).toInt();
        }

        if(counter == 1)
        {
            goto insert;
        }
    }

    insert:
        QString sql_string = "SELECT * FROM TBL_ATTENDANCE WHERE OUT_TIME != 0 AND SYNC_OUT IS NULL";
        sync_out_query.prepare(sql_string);

        if(sync_out_query.exec())
        {
            while(sync_out_query.next())
            {
                // --------------------------
                // post data to remote server
                // --------------------------
                remote_query.prepare("UPDATE credit_hours SET check_in = :check_in, check_out = :check_out WHERE id = :id");

                remote_query.bindValue(":id", record_m_id);
                remote_query.bindValue(":conference_id", conference_id);
                remote_query.bindValue(":session_id", session_id);
                remote_query.bindValue(":check_in", sync_out_query.value(3).toDateTime());
                remote_query.bindValue(":check_out", sync_out_query.value(4).toDateTime());

                if(remote_query.exec())
                {
                    print_screen("Successfuly synced, record ID: " + sync_out_query.value(0).toString());

                    // ---------------------------------
                    // change local server SYNC_IN to 1
                    // successfuly synced this record
                    // ---------------------------------
                    updateSyncOUTDataID(sync_out_query.value(0).toInt());
                } else { print_screen("Could not upadte record to remote server " + remote_query.lastError().text());}
            }
        } else { print_screen("Could not get record from local server " + remote_query.lastError().text());}
}

int MainWindow::refrenceToMemberID(int ref_number)
{
    QString sql_string = "SELECT * FROM TBL_ATTENDANCE WHERE OUT_TIME != 0 AND SYNC_OUT IS NULL";
    sync_out_query.prepare(sql_string);

    if(sync_out_query.exec())
    {
        while(sync_out_query.next())
        {
            // --------------------------
            // post data to remote server
            // --------------------------
            remote_query.prepare("SELECT member_id FROM member_con WHERE ref_number = :ref_number AND conference_id = :conference_id");

            remote_query.bindValue(":ref_number", ref_number);
            remote_query.bindValue(":conference_id", conference_id);

            if(remote_query.exec())
            {
                return sync_out_query.value(0).toInt();

            } else { print_screen("Could not upadte record to remote server " + remote_query.lastError().text());}
        }
    } else { return false; }
}

void MainWindow::updateSyncINDataID(int record_id)
{
    QString sql_string = "UPDATE TBL_ATTENDANCE SET SYNC_IN = :SYNC_IN WHERE ID = :id";
    update_query.prepare(sql_string);
    update_query.bindValue(":SYNC_IN", 1);
    update_query.bindValue(":id", record_id);

    if(update_query.exec())
    {
        print_screen("Successfuly updated synced data.");
    }
}

void MainWindow::updateSyncOUTDataID(int record_id)
{
    QString sql_string = "UPDATE TBL_ATTENDANCE SET SYNC_OUT = :SYNC_OUT WHERE ID = :id";
    update_query.prepare(sql_string);
    update_query.bindValue(":SYNC_OUT", 1);
    update_query.bindValue(":id", record_id);

    if(update_query.exec())
    {
        print_screen("Successfuly updated synced data.");
    }

}

void MainWindow::startTimer()
{
    print_screen("Starting timer for syncing data.");
    // get time
    getSyncTime();

    time_timer = sync_time * 60000;

    // start sync data
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(syncData()));
    timer->start(time_timer);

}

void MainWindow::stopTimer()
{
    timer->stop();
}



void MainWindow::insertDebugData()
{
    int count = 15;
    /*
    local_query.prepare("INSERT INTO TBL_ATTENDANCE "
                         "(MEMBER_ID, CONFERENCE_ID, INT_TIME, OUT_TIME, INOUT_STATUS, ATTENDANCE_DATE, WINNER, CATEGORY, LOGIN_TYPE, SESSION_ID, STALL_ID, SYNC_IN, SYNC_OUT) "
                         "VALUES (:MEMBER_ID, :CONFERENCE_ID, :INT_TIME, :OUT_TIME, :INOUT_STATUS, :ATTENDANCE_DATE, :WINNER, :CATEGORY, :LOGIN_TYPE, :SESSION_ID, :STALL_ID, :SYNC_IN, :SYNC_OUT)");
    */
    local_query.prepare("INSERT INTO TBL_ATTENDANCE "
                         "(MEMBER_ID, CONFERENCE_ID, INT_TIME, INOUT_STATUS, ATTENDANCE_DATE, WINNER, CATEGORY, LOGIN_TYPE, SESSION_ID, STALL_ID) "
                         "VALUES (:MEMBER_ID, :CONFERENCE_ID, :INT_TIME, :INOUT_STATUS, :ATTENDANCE_DATE, :WINNER, :CATEGORY, :LOGIN_TYPE, :SESSION_ID, :STALL_ID)");

    for(int i = 1; i < count; i++)
    {
        local_query.bindValue(":MEMBER_ID", "0");
        local_query.bindValue(":CONFERENCE_ID", "19");
        local_query.bindValue(":INT_TIME", QDateTime::currentDateTime());
        //local_query.bindValue(":OUT_TIME", NULL);
        local_query.bindValue(":INOUT_STATUS", "0");
        local_query.bindValue(":ATTENDANCE_DATE", "");
        local_query.bindValue(":WINNER", "0");
        local_query.bindValue(":CATEGORY", debug_number);
        local_query.bindValue(":LOGIN_TYPE", "0");
        local_query.bindValue(":SESSION_ID", "9");
        local_query.bindValue(":STALL_ID", "0");

        if(local_query.exec())
        {
            print_screen("Successful inserting to local database. ");
        } else {
            print_screen("Error!, inserting to database faild.");
        }
    }
}

void MainWindow::removeDebugData()
{
    local_query.prepare("DELETE FROM TBL_ATTENDANCE WHERE CATEGORY = :category");
    local_query.bindValue(":category", debug_number);
    if(local_query.exec())
    {
        print_screen("Successful removing data in database. ");
    } else {
       print_screen("Error!, removing data from database faild.");
    }
}

void MainWindow::print_screen(QString args)
{
    ui->txt_events_log->append( "[" + QDateTime::currentDateTime().toString() + "] " + args);
}

void MainWindow::pluginsLoader()
{
    //=============================================================
    // Database Connection
    //=============================================================
    QString outputDir = QDir::currentPath(); // with "/" separators

    QDir plugins( QCoreApplication::applicationDirPath() );
    plugins.cd( "plugins" );
    //qDebug() << plugins.absolutePath();

    QCoreApplication::addLibraryPath( plugins.absolutePath() );

}

void MainWindow::on_update_sync_time_clicked()
{
    setSyncTime(ui->sync_time->text().toInt());
}

void MainWindow::on_update_local_clicked()
{
    server_name_local = ui->txt_server_name_local->text();
    username_local = ui->txt_username_local->text();
    password_local = ui->txt_password_local->text();
    database_name_local = ui->txt_database_name_local->text();

    setLocalData();
}

void MainWindow::on_update_remote_clicked()
{
    server_name_remote = ui->txt_server_name_remote->text();
    username_remote = ui->txt_username_remote->text();
    password_remote = ui->txt_password_remote->text();
    database_name_remote = ui->txt_database_name_remote->text();

    setRemoteData();
}

void MainWindow::on_actionReconnect_to_servers_triggered()
{
    // config data init
    configDatabaseConnection();
    getSyncTime(); // get sync time
    getLocalData(); // local database string
    getRemoteData(); // remote database string

    // local data handler
    localDatabaseConnection();

    // handleing remote connection
    // ===========================
    remoteDatabaseConnection();


    // start timer for sync
    startTimer();
}

void MainWindow::on_connect_remote_db_clicked()
{
    // handleing remote connection
    // ===========================
    remoteDatabaseConnection();
}

void MainWindow::on_connect_local_db_clicked()
{
    // local data handler
    localDatabaseConnection();
}

void MainWindow::on_actionInsert_Test_Data_triggered()
{
    this->insertDebugData();
}

void MainWindow::on_actionRemove_Test_Data_triggered()
{
    this->removeDebugData();
}

void MainWindow::on_start_sync_clicked()
{
    ui->start_sync->setDisabled(true);
    ui->stop_sync->setDisabled(false);
    this->startTimer();
}

void MainWindow::on_stop_sync_clicked()
{
    ui->stop_sync->setDisabled(true);
    ui->start_sync->setDisabled(false);

    print_screen("Stoping timer for syncing data.");
    this->stopTimer();
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::on_clear_logs_bt_clicked()
{
    ui->txt_events_log->setText("");
}

void MainWindow::on_remote_config_bt_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);

    QRect pixRect = QRect(QPoint(0, 0), ui->remote_db_config_2->size());
    pixRect.moveCenter(QPoint( width()/2, 0 ));

    m_transition_remote->setDuration(300);
    m_transition_remote->setStartValue(QPoint(-(ui->remote_db_config_2->width()), ui->remote_db_config_2->y()));
    m_transition_remote->setEndValue( QPoint(pixRect.x(), ui->remote_db_config_2->y()) );
    m_transition_remote->setEasingCurve(QEasingCurve::OutQuad);
    //m_transition_remote->start();

}

void MainWindow::on_local_config_bt_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);

    QRect pixRect = QRect(QPoint(0, 0), ui->local_db_config_2->size());
    pixRect.moveCenter(QPoint( width()/2, 0 ));

    m_transition_local->setDuration(300);
    m_transition_local->setStartValue(QPoint(-(ui->local_db_config_2->width()), ui->local_db_config_2->y()));
    m_transition_local->setEndValue( QPoint(pixRect.x(), ui->local_db_config_2->y()) );
    m_transition_local->setEasingCurve(QEasingCurve::OutQuad);
    //m_transition_local->start();

}

void MainWindow::on_update_conference_session_bt_clicked()
{
    conference_name = ui->txt_conference_name->text();
    conference_id = ui->txt_conference_id->text().toInt();
    session_name = ui->txt_session_name->text();
    session_id = ui->txt_session_id->text().toInt();

    setConferenceData();
}
