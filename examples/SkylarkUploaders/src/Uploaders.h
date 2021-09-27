#ifndef _WebUOGL_SkylarkApp_h_
#define _WebUOGL_SkylarkApp_h_
#include <Skylark/Skylark.h>
#include <time.h>
#include <iostream>
#define NOAPPSQL
#include <plugin/sqlite3/Sqlite3.h>
namespace Upp{
	#define MODEL <SkylarkUploaders/ressources/db/AllowedUser.sch>
	#define SCHEMADIALECT  <plugin/sqlite3/Sqlite3Schema.h>
	#include "Sql/sch_header.h"
}
namespace Upp{
	class User : public Upp::Moveable<User>{
		private:
			int id;
			String login;
			String password;
			int right;
		public:
			User(const String& _login,const String& _password){ login = _login; password = _password;right = 0;}
			User(int _id,const String& _login,const String& _password){id = _id; login = _login; password = _password;right = 0;}
			User(int _id,const String& _login,const String& _password,int _right){id = _id; login = _login; password = _password;right = _right;}
			
			User& SetID(int _id){id = _id; return *this;}
			User& SetPassword(const Upp::String& _password){password = _password;return *this;}
			int GetId(){return id;}
			int GetRight(){return right;}
			const Upp::String& GetLogin(){return login;}
			
			bool IsValideUser(const String& loginValue,const String& passwordValue){ //Return true if user if data correspond
				if(login.IsEqual(loginValue) && password.IsEqual(passwordValue)) return true;
				return false;
			}
	};
	
	class File : public Upp::Moveable<File>{
		public:
			int id;
			User* owner = nullptr;
			Upp::String FilePath ="";
			Upp::String FileName ="";
			double FileSize = 0;
			
			File(){}
			File(const File& f){id = f.id; owner =f.owner; FilePath = f.FilePath; FileName = f.FileName; FileSize = f.FileSize;}
			File(int _id, User* _owner,const Upp::String& _FilePath,const Upp::String& _FileName){id = _id; owner = _owner; FilePath = _FilePath; FileName = _FileName;}
			File(int _id, User* _owner,const Upp::String& _FilePath,const Upp::String& _FileName, double _FileSize){id = _id; owner = _owner; FilePath = _FilePath; FileName = _FileName;FileSize = _FileSize;}
	
			void RemoveTheFile(){
				if(FileExists(FilePath)){
					remove(FilePath);
				}
			}
	};

	class FileURL : public Upp::Moveable<FileURL>{
		public:
			int Id;
			File* file = nullptr;	//The file focused by URL
			User* user = nullptr; //The user who created the URL
			
			String URL ="";
			int TotalDownloadNumber = 1;
			int DownloadNumber = 1;
			
			FileURL(){}
			FileURL(int _id, File* _file, User* _user, String _URL, int _TotalDownloadNumber, int _DownloadNumber=0){Id = _id; file = _file; user = _user; URL = _URL; TotalDownloadNumber = _TotalDownloadNumber; DownloadNumber = _DownloadNumber;}
			FileURL(const FileURL& f){Id = f.Id; file = f.file; user = f.user; URL = f.URL; TotalDownloadNumber = f.TotalDownloadNumber; DownloadNumber = f.DownloadNumber;}
	};
	
	struct Uploading : public Upp::Moveable<Uploading>{
		Upp::String Username; //username of user actually uploading
		Upp::String FileName;
		Upp::String FilePath;
		Upp::String Result;// holding result of uploading
		double FileSize = 0;
		int ActualChunkNumber = 0;
		int TotalChunkNumber = 0;
		FileOut f; // File out to the file
		time_t SysTimeLastRequest;

		void RemoveTheFile(){
			if(f)f.Close();
			if(FileExists(FilePath)){
				remove(FilePath);
			}
		}
	};
	
	struct Uploaders : SkylarkApp {
		private:
			void Init(int _port = 7979);
			void PrepareOrLoadBDD();
			void LoadAllUsers();
			void LoadAllFiles();
			void LoadAllURL();
			void AskForFirstUser(Upp::String& username, Upp::String& password); //Ask by the prompt a user name and password to the user. Linux/Window compliant
			
			Upp::String GeneralLink(); //Generate a short link
			
			void SynchroniseData(); //SynchroniseData function look up for data in a folder and ensure to update database with all new or deleted data
			Thread FileChecker; //The thread will ensure that database and memory is synchronised with folder data
		public:
			Sqlite3Session dbuser; //DataBase
			bool bddLoaded =false;
			Upp::VectorMap<Upp::String, User> AllUsers; //all user of database is stored in memory
			Upp::Vector<File> AllFiles; //Carrying all File
			Upp::Vector<FileURL> AllURL;
			Upp::Vector<Uploading> Uploadings; //carrying all current uploading
			
			int MAX_PACKET_SIZE = 20000;
			int TIMEOUT_UPLOAD_S = 10;
#if defined(PLATFORM_WIN32)
			Upp::String PathToUploadFolder = "C:\\Upp\\Uploaded";
#elif defined(PLATFORM_POSIX)
			Upp::String PathToUploadFolder = "~/Uploaded";
#else
#error Uploaders can only be compiled by using POSIX Path or Windows Path, Ensure compilation flag : PLATFORM_POSIX or PLATFORM_WIN32 is set
#endif
			
			Uploaders(); //Default constructor
			Uploaders(int _port);
			virtual ~Uploaders();
				
			const Upp::String& GetPath(){return path;}
			bool CreateUser(Upp::String login, Upp::String password , Upp::String& result);
			bool UpdatePassword(int id, Upp::String newPass, Upp::String& result);
			bool IsDocumentNameExist(Upp::String& name);
			bool CreateDocument(const Upp::String& name,const Upp::String& path,const Upp::String& username, int64 FileSize, Upp::String& result);
			bool CreateDocument(const Upp::String& name,const Upp::String& path,User& user, int64 FileSize, Upp::String& result);
			bool RemoveDocument(int ID);
			bool GenerateURL(int FileID,String username, int NumberTotalOfDownload);
			bool RemoveURL(int ID);
			
			User* RetrieveOwnerByUserID(int _ID);
			User* RetrieveUserByUsername(const Upp::String& userName);
			User* IsValideUser(const String& loginValue,const String& passwordValue);
			User* RetrieveSuperUserOrOneUser();
			
			File* FindFileByName(const Upp::String& name);
			File* FindFileByID(int ID);
			
			FileURL* FindFileURLByID(int ID);
			bool RemoveAllURLFromFileID(int ID);
	};
}
#endif