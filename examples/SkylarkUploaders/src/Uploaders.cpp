#include "Uploaders.h"
#if defined(PLATFORM_WIN32)
	#include <windows.h>
#elif defined(PLATFORM_POSIX)
	#include <termios.h>
	#include <unistd.h>
#endif
namespace Upp{
	
#include <Sql/sch_schema.h>
#include <Sql/sch_source.h>

Uploaders::Uploaders(){
	Init();
}

Uploaders::Uploaders(int _port){
	Init(_port);
}
void Uploaders::Init(int _port){
#ifdef _DEBUG
	prefork = 0;
	use_caching = false;
#endif
	threads = 10;
	root = ""; //Root of our app
	port = _port; //Port of our app
	path = GetFileDirectory(__FILE__); //Working directory
	if(!DirectoryExists(path +"../ressources")){
		path = GetExeFolder();
		if(!DirectoryExists(path +"/ressources")){
			Cout() << "ressources directory is absent. Ensure the ressource directory provided with the source code is present near the application executable" << EOL;
			Exit(-1);
		}
	}
	/* By using this assertion we ensure every path given in our class are valide */
	if(!DirectoryExists(PathToUploadFolder)){
		PathToUploadFolder = GetExeFolder() + "/uploaded";
		DirectoryCreate(PathToUploadFolder);
		
	}
	PrepareOrLoadBDD(); //Load BDD
	/* We ensure Database have correctly been load */
	ASSERT_(bddLoaded, "Error, Database coming with this app have not been build correctly");
	LoadAllUsers(); //we now load all user existing in database in our memory
	LoadAllFiles(); //Same for file path
	LoadAllURL(); //Same for URL enabled
	/* Now it's time to launch a thread that check for each file every minute until server is shuting down,
	   It will update the database and memory depending of wich file have been deleted. */
	SynchroniseData();
	FileChecker.Run([&](){
		for(;;){
			for(int e = 0;e< TIMEOUT_UPLOAD_S; e++){
				if(Thread::IsShutdownThreads())break;
				Sleep(1000);
			}
			if(Thread::IsShutdownThreads())break;
			SynchroniseData();
		}
	});
}

Uploaders::~Uploaders(){
	Thread::ShutdownThreads();
}
void Uploaders::SynchroniseData(){
	Vector<File> AllFilesCopy;
	for(File& f : AllFiles){
		AllFilesCopy.Create(f);
	}
	time_t systime;
	time(&systime);

	int cpt3 = 0;
	Vector<int> ToDelete;
	for(Uploading& up : Uploadings){
		LOG(up.Username + " timeDif : " + AsString(systime - up.SysTimeLastRequest));
		if((systime - up.SysTimeLastRequest)  > TIMEOUT_UPLOAD_S){
			ToDelete.Add(cpt3);
		}
		cpt3++;
	}
	for(int e = 0; e < ToDelete.GetCount(); e++){
		LOG(Uploadings[e].Username + " uploading data have been remove");
		Uploadings[e].RemoveTheFile();
		Uploadings.Remove(e,1);
		for(int& r : ToDelete){
			r--;
		}
	}
	Upp::String dummyStr="";
	FindFile ff(PathToUploadFolder );
	ff.Search(AppendFileName(PathToUploadFolder,"/*.*"));
	do{
		if( !ff.GetName().IsEqual(".") && ! ff.GetName().IsEqual("..")){
			File* f = FindFileByName(ff.GetName());
			if(!f){
				if(Uploadings.GetCount() == 0){
					LOG("Synchronize : new data have been found : " + ff.GetName());
					//The File have not been found and must be added
					User* su = RetrieveSuperUserOrOneUser();
					if(su) CreateDocument(ff.GetName(),ff.GetPath(),*su,ff.GetLength(),dummyStr);
					else LOG("ERREUR on File Synchronisation process, Not user have been found on dataBase");
				}
			}else{
				//the file have been found, we can now delete him from AllFilesCopy
				int cpt =0;
				for(File& f1 : AllFilesCopy){
					if(f1.id == f->id)
						AllFilesCopy.Remove(cpt);
					cpt++;
				}
			}
		}
	}while(ff.Next());
	
	//here we should have only the file wich don't exist anymore in folder stored in
	//AllFilesCopy. We must iterate through them and remove it from database
	
	int cpt = 0;
	do{
		if(AllFilesCopy.GetCount() >0){
			LOG("Synchronize : one file is unfindable and must be delete : " + AllFilesCopy[cpt].FileName);
			RemoveDocument(AllFilesCopy[cpt].id);
			AllFilesCopy.Remove(cpt);
		}else break;
	}while(AllFilesCopy.GetCount() > 0);
}

File* Uploaders::FindFileByName(const Upp::String& name){
	for(File& f : AllFiles){
		if(f.FileName.IsEqual(name)){
			return &f;
		}
	}
	return nullptr;
}
bool Uploaders::RemoveDocument(int ID){
	Sql sql(dbuser);
	if(sql*Delete(FILES_UPLOADED).Where(ID_FILE == ID)){
		int cpt = 0;
		for(File& f : AllFiles){
			if(f.id == ID){
				RemoveAllURLFromFileID(f.id);
				f.RemoveTheFile();
				AllFiles.Remove(cpt);
				return true;
			}
			cpt++;
		}
	}
	return false;
}

void Uploaders::AskForFirstUser(Upp::String& username, Upp::String& password){ //Ask by the prompt a user name and password to the user. Linux/Window compliant
	username ="";
	password ="";
	Upp::String dummy="";
#if defined(PLATFORM_WIN32)
	while(TrimBoth(username).GetCount() == 0){
		Cout() << "Enter username for default user :" << EOL;
		username = ReadStdIn();
	}
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));
	while(TrimBoth(password).GetCount() ==0 && TrimBoth(dummy).GetCount() == 0 && TrimBoth(password).IsEqual(TrimBoth(dummy))){
		while(TrimBoth(password).GetCount() == 0){
			Cout() << "Enter password for "+  username +" :" << EOL;
			password = ReadStdIn();
		}
		while(TrimBoth(dummy).GetCount() == 0){
			Cout() << "Re enter same password for "+  username +" :" << EOL;
			dummy = ReadStdIn();
		}
		if(!TrimBoth(password).IsEqual(TrimBoth(dummy))){
			Cout() << "No Matching between passwords" << EOL;
			password ="";
			dummy ="";
		}
    }
	SetConsoleMode(hStdin, mode);
#elif defined(PLATFORM_POSIX)
	while(TrimBoth(username).GetCount() == 0){
		Cout() << "Enter username for default user :" << EOL;
		username = ReadStdIn();
	}
	termios oldt;
	tcgetattr(STDIN_FILENO, &oldt);
	termios newt = oldt;
	newt.c_lflag &= ~ECHO;
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    while(TrimBoth(password).GetCount() ==0 && TrimBoth(dummy).GetCount() == 0 && TrimBoth(password).IsEqual(TrimBoth(dummy))){
		while(TrimBoth(password).GetCount() == 0){
			Cout() << "Enter password for "+  username +" :" << EOL;
			password = ReadStdIn();
		}
		while(TrimBoth(dummy).GetCount() == 0){
			Cout() << "Re enter same password for "+  username +" :" << EOL;
			dummy = ReadStdIn();
		}
		if(!TrimBoth(password).IsEqual(TrimBoth(dummy))){
			Cout() << "No Matching between passwords" << EOL;
			password ="";
			dummy ="";
		}
    }
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
}
void Uploaders::PrepareOrLoadBDD(){
	if(!bddLoaded){
		dbuser.LogErrors(true);
		bool mustCreate = false;
		Upp::String path =  this->path + "../ressources/db/allowedUser.db";
		if(!FileExists(path))mustCreate =true;
		if(!dbuser.Open(path)){
			SKYLARKLOG("Can't create or open database file");
			bddLoaded = false;
			return;
		}
		dbuser.LogErrors();
		if(mustCreate){
			SQL = dbuser;
			SqlSchema sch(SQLITE3);
			All_Tables(sch);
			SqlPerformScript(sch.Upgrade());
			SqlPerformScript(sch.Attributes());
			sch.SaveNormal();
			 //Enable foreign key resolution on SQL lite
			Sql sql(dbuser);
			sql.Execute("PRAGMA foreign_keys = ON;"); //Enable Foreign keys
			//Prepare creation of the first user
			Upp::String login,password;
			AskForFirstUser(login,password);
			if(password.GetCount() > 0 && login.GetCount()){
				if(sql*Insert(USERS_ALLOWED)(LOGIN,login)(PASS,SHA256String(password+"="+login))(RIGHT,1)){
					LOG("Default user named " + login + " Have been created");
				}else{
					LOG("Failed to create default user named " + login);
				}
			}
		}
		bddLoaded = true;
		#undef MODEL
	}
}
bool Uploaders::CreateUser(Upp::String login, Upp::String password, Upp::String& result){
	Sql sql(dbuser);
	if(sql*Insert(USERS_ALLOWED)(LOGIN,login)(PASS,password)){
		AllUsers.Add(login, User(sql.GetInsertedId().Get<int64>(), login, password,0));
		return true;
	}else{
		result = "SQL insertion have failled";
		return false;
	}
}
bool Uploaders::IsDocumentNameExist(Upp::String& name){
	for(File& f : AllFiles){
		if(f.FileName.IsEqual(name)) return true;
	}
	return false;
}
bool Uploaders::CreateDocument(const Upp::String& name,const Upp::String& path,const Upp::String& userName, int64 FileSize, Upp::String& result){
	User* us =RetrieveUserByUsername(userName);
	if(us){
		return CreateDocument(name,path,*us,FileSize,result);
	}else{
		result = name + " uploading have failled : user is unknow";
		return false;
	}
}
bool Uploaders::CreateDocument(const Upp::String& name,const Upp::String& path,User& user, int64 FileSize, Upp::String& result){
	Sql sql(dbuser);
	if(sql*Insert(FILES_UPLOADED)(FILE_OWNER,user.GetId())(PATH,path)(NAME,name)(FILE_SIZE,FileSize)){
		AllFiles.Add(File(sql.GetInsertedId().Get<int64>(),&user,path,name,FileSize));
		return true;
	}else{
		result = name + " uploading have failled : Sql insertion have failled";
		return false;
	}
}
bool Uploaders::GenerateURL(int FileID, String username, int NumberTotalOfDownload){
	File* f =FindFileByID(FileID);
	User* us = RetrieveUserByUsername(username);
	if(f && us){
		Sql sql(dbuser);
		Upp::String urlGenerated = GeneralLink();
		if(sql*Insert(FILES_URL)(FILE,f->id)(CREATOR,us->GetId())(URL,urlGenerated)(DOWNLOAD_NUMBER,NumberTotalOfDownload)){
			AllURL.Create(sql.GetInsertedId().Get<int64>(),f,us,urlGenerated,NumberTotalOfDownload,0);
			return true;
		}else{
			return false;
		}
	}else{
		return false;
	}
	return false;
}

Upp::String Uploaders::GeneralLink(){
	//trivial way of generating URL depending on systime
	time_t systime;
	time(&systime);
	int Mask = 0x000000FF;
	Upp::Vector<char> theLink;
	while(systime){
		int test =(systime & Mask);
		systime = systime >> 8;
		if( test < 65 ){
			test = 25.0f *(((float)test)/64.0f);
			test += 65;
		}else if(test > 123){
			test = 25.0f *(((float)(test-123))/132.0f);
			test += 97;
		}else if(test > 90 && test < 97){
			test = 25.0f *(((float)(test-90))/7.0f);
			test += 97;
		}
		theLink.Add(test);
	}
	Upp::String Url = "";
	for(char& c : theLink){
		Url << c;
	}
	for(int e = theLink.GetCount()-1;e>= 0; e--){
		Url << theLink[e];
	}
	return Url;
}

bool Uploaders::UpdatePassword(int id, Upp::String newPass, Upp::String& result){
	Sql sql(dbuser);
	if(Update(USERS_ALLOWED)(PASS,newPass).Where(ID_USER == id)){
		for(User& us : AllUsers){
			if(us.GetId() == id){
				us.SetPassword(newPass);
			}
		}
		return true;
	}else{
		result = "SQL update have failled";
		return false;
	}
}
bool Uploaders::RemoveURL(int ID){
	Sql sql(dbuser);
	if(sql*Delete(FILES_URL).Where(ID_URL == ID)){
		int cpt = 0;
		for(FileURL& f : AllURL){
			if(f.Id == ID){
				AllURL.Remove(cpt);
				return true;
			}
			cpt++;
		}
	}
	return false;
}
void Uploaders::LoadAllUsers(){
	AllUsers.Clear();
	Sql sql(dbuser);
	sql*Select(SqlAll()).From(USERS_ALLOWED);
	while(sql.Fetch()){
		if(AllUsers.Find(sql[1]) == -1){
			AllUsers.Add(sql[1], User(sql[0], sql[1], sql[2],sql[3]));
		}
	}
}
void Uploaders::LoadAllFiles(){
	AllFiles.Clear();
	Sql sql(dbuser);
	sql*Select(SqlAll()).From(FILES_UPLOADED);
	while(sql.Fetch()){
		AllFiles.Create(sql[0],RetrieveOwnerByUserID(sql[1]),sql[3],sql[4],sql[2].Get<double>());
	}
}
void Uploaders::LoadAllURL(){
	AllURL.Clear();
	Sql sql(dbuser);
	sql*Select(SqlAll()).From(FILES_URL);
	while(sql.Fetch()){
		AllURL.Create(sql[0], FindFileByID(sql[1]), RetrieveOwnerByUserID(sql[2]), sql[3], sql[4], sql[4]);
	}
}

User* Uploaders::RetrieveUserByUsername(const Upp::String& userName){
	for(User& us : AllUsers){
		if(us.GetLogin().IsEqual(userName)) return &us;
	}
	return nullptr;
}
User* Uploaders::RetrieveOwnerByUserID(int ID){
	if(ID > -1){
		for(User& us : AllUsers){
			if(us.GetId() == ID){
				return &us;
			}
		}
	}
	return nullptr;
}
User* Uploaders::RetrieveSuperUserOrOneUser(){
	for(User& us : AllUsers){
		if(us.GetRight() == 1){
			return &us;
		}
	}
	if(AllUsers.GetCount() > 0)return &AllUsers[0];
	else return nullptr;
}
User* Uploaders::IsValideUser(const String& loginValue,const String& passwordValue){
	for(const Upp::String& key : AllUsers.GetKeys()){
		if( key.IsEqual(loginValue)){
			if(AllUsers.Get(key).IsValideUser(loginValue,passwordValue)){
				return &(AllUsers.Get(key));
			}else{
				return nullptr;
			}
		}
	}
	return nullptr;
}
File* Uploaders::FindFileByID(int ID){
	for(File& f : AllFiles){
		if(f.id == ID){
			return &f;
		}
	}
	return nullptr;
}
FileURL* Uploaders::FindFileURLByID(int ID){
	for(FileURL& f : AllURL){
		if(f.Id == ID){
			return &f;
		}
	}
	return nullptr;
}
bool Uploaders::RemoveAllURLFromFileID(int ID){
	Vector<int> v;
	int cpt =0;
	for(FileURL& fu : AllURL){
		if(fu.file && fu.file->id == ID){
			RemoveURL(fu.Id);
			v.Add(cpt);
		}
		cpt++;
	}
	return true;
}
}