#include "ResHandler.hpp"
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

void ResHandler::parse_buf(char *buf, std::string &filename, std::string &content_type)
{
	std::stringstream ss(buf);
	std::string word;

	while (ss >> word)
	{
		if (word.size() > 9 && word.substr(0, 9) == "filename=")
			filename = word.substr(9);
		if (word.size() == 13 && word.substr(0, 13) == "Content-Type:")
			ss >> content_type;
	}
}

void parse_post(char *buf, std::string &filename)
{
	std::stringstream ss(buf);
	std::string word;

	while (ss >> word)
	{
		if (word.size() > 9 && word.substr(0, 9) == "filename=")
			filename = word.substr(9);
	}
}

std::string removeAll(std::string str, const std::string &from)
{
	size_t start_pos = 0;
	while ((start_pos = str.find(from)) != std::string::npos)
	{
		str.erase(start_pos, from.length());
	}
	return str;
}

bool ResHandler::CheckifPathisFile(const char *path)
{
	struct stat s;
	if (stat(path, &s) == 0)
	{
		if (s.st_mode & S_IFDIR)
			return false; // it's a director
		else if (s.st_mode & S_IFREG)
			return true; // it's a file
		else
			return true;
	}
	return true;
}

void ResHandler::setDate(void)
{
	struct tm *Time;
	char stock[100];
	struct timeval timeval;

	gettimeofday(&timeval, NULL);
	Time = gmtime(&timeval.tv_sec);
	strftime(stock, 100, "%a, %d %b %Y %H:%M:%S GMT", Time);
	MainServer.time = std::string(stock);
}

void ResHandler::CheckModiDate(void)
{
	char src[100];
	struct stat status;
	struct tm *That;

	if (stat(MainServer.path.c_str(), &status) == 0)
	{
		That = gmtime(&status.st_mtime);
		strftime(src, 100, "%a, %d %b %Y %H:%M:%S GMT", That);
		MainServer.Modiftime = std::string(src);
	}
}

void ResHandler::GetContent_Type(std::string path)
{
	MainServer.content_type = path.substr(path.rfind(".") + 1, path.size() - path.rfind("."));
	MainServer.cgitype = MainServer.content_type;
	if (MainServer.content_type == "html")
		MainServer.content_type = "text/html";
	else if (MainServer.content_type == "pdf")
		MainServer.content_type = "application/pdf";
	else if (MainServer.content_type == "xml")
		MainServer.content_type = "text/xml";
	else if (MainServer.content_type == "js")
		MainServer.content_type = "text/javascript";
	else if (MainServer.content_type == "css")
		MainServer.content_type = "text/css";
	else if (MainServer.content_type == "jpeg" || MainServer.content_type == "jpg" || MainServer.content_type == "pjp" || MainServer.content_type == "jfif" || MainServer.content_type == "pjpeg")
		MainServer.content_type = "image/jpeg";
	else if (MainServer.content_type == "png")
		MainServer.content_type = "image/png";
	else if (MainServer.content_type == "mp4")
		MainServer.content_type = "video/mp4";
	else if (MainServer.content_type == "webm")
		MainServer.content_type = "video/webm";
	else if (MainServer.content_type == "mpeg")
		MainServer.content_type = "video/mpeg";
	else if (MainServer.content_type == "mp3")
		MainServer.content_type = "audio/mpeg";
	else if (MainServer.content_type == "doc")
		MainServer.content_type = "application/msword";
	else if (MainServer.content_type == "gif")
		MainServer.content_type = "image/gif";
	else if (MainServer.content_type == "ico")
		MainServer.content_type = "image/x-icon";
	else if (MainServer.content_type == ".cpp")
		MainServer.content_type = "text/plain";
	else
		MainServer.content_type = "text/html";
}
template <typename T>
std::string ToString(T numb)
{
	std::stringstream stream;
	stream << numb;
	return stream.str();
}


void ResHandler::Methodes(std::string FileConf)
{
	ParserConfig cf(FileConf);
	MainServer.ConfigName = FileConf;
	std::vector<ServerMembers> servers = cf.getConfig();
	Server serv(servers);
	std::vector<ServerMembers>::iterator it = cf.getConfig().begin();
	std::vector<ServerMembers>::iterator ss = cf.getConfig().begin();
	std::vector<ServerMembers>::iterator xx = cf.getConfig().end();
	int j = 1;
	int jn = 1;
	std::string aled;
	std::string all;
	all = "";
	int num;
	bool yes;
	std::map<int, int> portfind;
	for (int pl = 1; ss != xx; ++ss, ++pl, jn++)
		portfind.insert(std::make_pair(ss->port, jn));
	std::vector<LocationMembers>::iterator locItos = it->locations.begin();
	for (int i = 1; i != portfind[atoi(MainServer.host.substr(MainServer.host.find(":") + 1).c_str())]; i++)
		it++;
	for (std::vector<LocationMembers>::iterator locIt = it->locations.begin(); locIt != it->locations.end(); ++locIt, locItos++, ++j)
	{
		if (MainServer.path.find(locIt->uri) != std::string::npos)
		{
			num = j;
			MainServer.uril_root = locIt->root;
			aled = locIt->uri;
			MainServer.max_body = locIt->max_body_size;
			MainServer.upload = it->upload;
			for (std::map<int, std::string>::iterator errIt = locIt->error_pages.begin(); errIt != locIt->error_pages.end(); ++errIt)
				MainServer.mp.insert(std::make_pair(ToString(errIt->first), errIt->second));
			all = "";
			for (std::set<std::string>::iterator namesIt = locItos->allowedMethods.begin(); namesIt != locItos->allowedMethods.end(); ++namesIt)
					all += *namesIt;
			yes = true;
		}
		else if (yes != true)
		{
			MainServer.max_body = it->max_body_size;
		}
		for (std::map<std::string, std::string>::iterator namesIt = it->cgis.begin(); namesIt != it->cgis.end(); ++namesIt)
			MainServer.mp.insert(std::make_pair(namesIt->first, namesIt->second));
		MainServer.upload = it->upload;
	}
	for (std::vector<std::string>::iterator namesIt = it->server_name.begin(); namesIt != it->server_name.end(); ++namesIt)
		MainServer.serv_name = *namesIt;
	std::map<std::string, std::string> indexmap;
	for (std::vector<LocationMembers>::iterator locIt = it->locations.begin(); locIt != it->locations.end(); ++locIt, ++j)
		for (std::vector<std::string>::iterator namesIt = locIt->index.begin(); namesIt != locIt->index.end(); ++namesIt)
			indexmap.insert(std::make_pair(locIt->uri, *namesIt));
	if (MainServer.path == aled)
		MainServer.path += indexmap[aled];
	MainServer.path.replace(MainServer.path.find(aled), aled.length(), MainServer.uril_root);
	TheReposn = "";
	if (MainServer.type == "GET" && all.find(MainServer.type) != std::string::npos)
		GetMethodes();
	else if (MainServer.type == "DELETE" && all.find(MainServer.type) != std::string::npos)
		DeleteMethodes();
	else if (MainServer.type == "POST" && all.find(MainServer.type) != std::string::npos)
		POSTMethodes();
	else
	{
		reseat();
		if (MainServer.type == "POST" || MainServer.type == "DELETE" || MainServer.type == "GET")
			MainServer.code = 405;
		else
			MainServer.code = 501;
		ErrorPage();
		TheReposn += MainServer.protocol + " " + ToString(MainServer.code) + " " + EroRep[MainServer.code];
		TheReposn += "\nDate : " + MainServer.time;
		TheReposn += "\nServer: Webserv /1.0.0";
		TheReposn += "\n\n";
		TheReposn += MainServer.http;
	}
}

int ResHandler::filexist(const char *fileName)
{
	std::string line;
	MainServer.code = 200;
	ParserConfig cf(MainServer.ConfigName);
	int autoindx[65535];
	std::vector<ServerMembers>::iterator ite = cf.getConfig().end();
	std::vector<ServerMembers>::iterator it = cf.getConfig().begin();
	for (int i = 1; it != ite; ++it, ++i)
	{
		autoindx[it->port] = it->autoindex;
	}
	std::ifstream document;
	if (CheckifPathisFile(fileName) == true)
	{
		if ((access(fileName, F_OK) == 0))
		{
			document.open(fileName, std::ifstream::in);
			if (document.is_open() == false)
			{
				MainServer.code = 403;
				return MainServer.code;
			}
			std::stringstream test;
			test << document.rdbuf();
			MainServer.http = test.str();
			if (MainServer.cgitype == "py" && MainServer.mp[".py"].find("/usr/bin/python") != std::string::npos)
			{
				MainServer.http = g.cgiExecute("/usr/bin/python", fileName, MainServer.postname, MainServer.postvalue, 0, MainServer.bando);
				MainServer.code = 200;
				return MainServer.code;
			}
			else if (MainServer.cgitype == "pl" && MainServer.mp[".pl"].find("/usr/bin/perl") != std::string::npos)
			{
				MainServer.http = g.cgiExecute("/usr/bin/perl", fileName, MainServer.postname, MainServer.postvalue, 0, MainServer.bando);
				MainServer.code = 200;
				return MainServer.code;
			}
			else if (MainServer.cgitype == "php" && MainServer.mp[".php"].find("/usr/bin/php") != std::string::npos)
			{
				MainServer.http = g.cgiExecute("/usr/bin/php", fileName, MainServer.postname, MainServer.postvalue, 0, MainServer.bando);
				MainServer.code = 200;
				return MainServer.code;
			}
			return MainServer.code;
		}
		else
			MainServer.code = 404;
		return MainServer.code;
	}
	else if (CheckifPathisFile(fileName) == false && MainServer.type == "GET" && autoindx[atoi(MainServer.host.substr(MainServer.host.find(":") + 1).c_str())] == 1)
	{
		MainServer.http += FautoIndex(fileName);
		MainServer.code = 200;
		return MainServer.code;
	}
	else
		MainServer.code = 404;
	return MainServer.code;
}
void ResHandler::ErrorPage(void)
{
	std::string line;
	std::ifstream fahd;
	std::string path;
	if (MainServer.mp[ToString(MainServer.code)] == "")
	{
		path = ToString(realpath(".", NULL)) + "/srcs/Http(Errors)/" + ToString(MainServer.code) + ".html";
	}
	else
		path = ToString(realpath(".", NULL)) + MainServer.mp[ToString(MainServer.code)];
	std::ifstream document;
	if ((access(path.c_str(), F_OK) == 0))
	{
		document.open(path, std::ios::in);
		while (getline(document, line))
		{
			MainServer.http += line + " ";
		}
	}
}

void ResHandler::Erostatus(void)
{
	EroRep[100] = "Continue";
	EroRep[200] = "OK";
	EroRep[201] = "Created";
	EroRep[204] = "No Content";
	EroRep[400] = "Bad Request";
	EroRep[403] = "Forbidden";
	EroRep[404] = "Not Found";
	EroRep[405] = "Method Not Allowed";
	EroRep[413] = "Payload Too Large";
	EroRep[500] = "Internal Server Error";
	EroRep[501] = "Not Implemented";
}

int ResHandler::GetMethodes(void)
{
	std::string raw_path = realpath(".", NULL);
	std::string path = realpath(".", NULL) + removeAll(MainServer.path, realpath(".", NULL));
	if (path.find(raw_path + raw_path) != std::string::npos)
		path = path.substr(path.find(raw_path));
	MainServer.path = path + "/";
	GetContent_Type(path);
	filexist(path.c_str());
	std::string testit;
	ErrorPage();
	ParseQueryString_(MainServer.bando.substr(MainServer.bando.find("\r\n\r\n") + strlen("\r\n\r\n")));
	if (MainServer.content_len >= MainServer.max_body)
	{
		MainServer.code = 413;
		MainServer.http = "";
		ErrorPage();
	}
	else
		MainServer.code = 200;
	std::stringstream test;
	if (MainServer.http.find("Status: 500") != std::string::npos)
	{
		MainServer.code = 500;
		MainServer.http = "";
		ErrorPage();
	}
	TheReposn += MainServer.protocol + " " + ToString(MainServer.code) + " " + EroRep[MainServer.code];
	TheReposn += "\nDate : " + MainServer.time;
	TheReposn += "\nServer: " + MainServer.serv_name;
	TheReposn += "\nLast-modified " + MainServer.Modiftime;
	TheReposn += "\nContent-Type: " + MainServer.content_type;
	TheReposn += "\nContent-Length : " + std::to_string(MainServer.http.size() - 1);
	TheReposn += "\nContent-Location: " + MainServer.path;
	TheReposn += "\nTransfer-Encoding: identity" + MainServer.encoding;
	TheReposn += "\n\n";
	TheReposn += MainServer.http;
	return 0;
}

void ResHandler::DeletePathHandler(void)
{
	std::string path = realpath(".", NULL) + MainServer.path;
	if (CheckifPathisFile(path.c_str()) == true)
	{
		if ((access(path.c_str(), F_OK) == 0))
		{
			if (remove(path.c_str()) == 0)
				MainServer.code = 204;
			else
				MainServer.code = 403;
		}
		else
			MainServer.code = 404;
	}
	else
		MainServer.code = 404;
}

void ResHandler::DeleteMethodes(void)
{
	DeletePathHandler();
	if (MainServer.content_len >= MainServer.max_body)
		MainServer.code = 413;
	TheReposn += MainServer.protocol + " " + ToString(MainServer.code) + " " + EroRep[MainServer.code];
	TheReposn += "\nDate : " + MainServer.time;
	TheReposn += "\n\n";
	reseat();
	ErrorPage();
	TheReposn += MainServer.http;
}
std::string ResHandler::Uploadit(std::string sear, std::string buffer)
{
	int check_prob = 0;
	std::string mainbuffer(buffer);
	size_t i = mainbuffer.rfind("filename=\"");
	if (i != std::string::npos)
	{
		i += 10;
		size_t j = mainbuffer.find("\"", i);
		if (j != std::string::npos)
		{
			mainbuffer = std::string((mainbuffer.begin() + i), mainbuffer.begin() + j);
		}
	}
	std::string hahiyaupload = "." + MainServer.upload;
	std::string root = "";
	int fd;
	if ("" == hahiyaupload)
		fd = open((hahiyaupload + "/" + mainbuffer).c_str(), O_RDWR | O_CREAT | O_TRUNC, 00777);
	else
	{
		mkdir((hahiyaupload + "/" + "").c_str(), 0755);
		fd = open((hahiyaupload + "/" + "" + "" + mainbuffer).c_str(), O_RDWR | O_CREAT | O_TRUNC, 00777);
	}
	if (fd == -1)
		return (std::string());
	int checkWrite = write(fd, sear.c_str(), sear.size());
	if (checkWrite == 0)
		check_prob = 0;
	else if (checkWrite == -1)
		check_prob = -1;
	close(fd);
	return (mainbuffer);
}
std::string get_right_of_delim(std::string const &str, std::string const &delim)
{
	return str.substr(str.find(delim) + delim.size());
}

void ResHandler::ParseQueryString_(const std::string &query_string)
{
	std::size_t position = 0;
	std::size_t i = 1;
	while (position < query_string.size())
	{
		const std::size_t next_delimiter = query_string.find('&', position);
		std::string query;
		if (next_delimiter == std::string::npos)
		{
			query = query_string.substr(position);
		}
		else
		{
			query = query_string.substr(position, next_delimiter - position);
		}
		const std::size_t pair_delimiter = query.find('=');
		const std::string name = query.substr(0, pair_delimiter);
		if (name.empty())
		{
			return;
		}
		std::string value;
		if (pair_delimiter != std::string::npos)
		{
			value = query.substr(pair_delimiter + 1);
		}
		MainServer.postname[i] = name;
		MainServer.postvalue[i] = value;
		if (next_delimiter == std::string::npos)
		{
			break;
		}
		position = next_delimiter + 1;
		i++;
	}
	MainServer.envj = i;
}

int ResHandler::POSTMethodes()
{
	std::string path = realpath(".", NULL) + removeAll(MainServer.path, realpath(".", NULL));
	MainServer.code = 413;
	ParseQueryString_(MainServer.bando.substr(MainServer.bando.find("\r\n\r\n") + strlen("\r\n\r\n")));
	GetContent_Type(path);
	if (MainServer.path.substr(MainServer.path.find_last_of(".") + 1) == "php")
		MainServer.http = g.cgiExecute("/usr/bin/php", path.c_str(), MainServer.postname, MainServer.postvalue, MainServer.envj, MainServer.bando);
	if (MainServer.path.substr(MainServer.path.find_last_of(".") + 1) == "py")
		MainServer.http = g.cgiExecute("/usr/bin/python", path.c_str(), MainServer.postname, MainServer.postvalue, MainServer.envj, MainServer.bando);
	bool upload = false;
	if (MainServer.content_len <= MainServer.max_body)
	{
		MainServer.code = 200;
		std::string sear(MainServer.bando);
		size_t i = sear.rfind("Content-Type:");
		if (i != std::string::npos)
			i = sear.find("\n", i);
		if (i != std::string::npos)
		{
			size_t j = sear.find("------WebKitFormBoundary", i);
			if (j != std::string::npos)
			{
				upload = true;
				MainServer.http = "<!DOCTYPE htm1l>\n<html>\n<h1>File " + MainServer.filename + "Hs Been Uploaded</h1>";
				MainServer.http = "<a href=" + MainServer.upload + removeAll(MainServer.filename, "\"") + " download>Download File</a>";
				Uploadit(std::string((sear.begin() + i + 3), sear.begin() + j - 2), MainServer.bando);
			}
		}
	}
	if (MainServer.content_len >= MainServer.max_body)
		MainServer.code = 413;
	if (MainServer.http.find("Status: 500") != std::string::npos)
	{
		MainServer.code = 500;
		MainServer.http = "";
		ErrorPage();
	}
	TheReposn += MainServer.protocol + " " + ToString(MainServer.code) + " " + EroRep[MainServer.code];
	TheReposn += "\nDate : " + MainServer.time;
	TheReposn += "\nServer: " + MainServer.serv_name;
	TheReposn += "\nContent-Type: " + MainServer.content_type;
	if (upload != true)
		TheReposn += "\nContent-Length : " + std::to_string(MainServer.http.size() - 1); // make a update for upload
	else
		TheReposn += "\nContent-Length : " + ToString(MainServer.content_len);
	TheReposn += "\nContent-Location: " + MainServer.path;
	TheReposn += "\n\n";
	ErrorPage();
	TheReposn += MainServer.http;
	return 0;
}

void ResHandler::reseat(void)
{
	MainServer.http = "";
}

std::string ResHandler::geit(std::string const &dirEntry, std::string Directory, std::string const &host)
{
	std::stringstream ss;
	if (dirEntry != ".." && dirEntry != ".")
		ss << "\t\t<p><a href=\"http://" + host << Directory + "/" + dirEntry + "\">" + dirEntry + "/" + "</a></p>\n";
	return ss.str();
}

std::string ResHandler::FautoIndex(const char *path)
{
	std::string Directory(MainServer.path);
	DIR *dir = opendir(path);
	std::string Autoindex_Page =
		"<!DOCTYPE html>\n\
    <html>\n\
    <head>\n\
            <title>" +
		Directory + "</title>\n\
    </head>\n\
    <body>\n\
    <h1>INDEX</h1>\n\
    <p>\n";
	if (dir == NULL)
	{
		std::cerr << "Error: could not open the following path" << path << std::endl;
		return "";
	}
	if (Directory[0] != '/')
		Directory = "/" + Directory;
	for (struct dirent *dirEntry = readdir(dir); dirEntry; dirEntry = readdir(dir))
	{
		Autoindex_Page += geit(std::string(dirEntry->d_name), Directory, MainServer.host);
	}
	Autoindex_Page += "\
    </p>\n\
    </body>\n\
    </html>\n";
	closedir(dir);
	return Autoindex_Page;
}
#include <unistd.h>
#include <fcntl.h>
int Server::run(std::string FileConf)
{
	fd_set read_fd_set;
	fd_set write_fd_set = {0};
	;
	int new_fd, ret_val, i;
	int check_probl = 0;
	socklen_t addrlen;
	int all_connections[MAX_CONNECTIONS];
	std::string kfe;

	for (int i = 0; i < getNbPort(); i++)
	{
		if (fd[i] == -1)
		{
			return -1;
		}
	}
	for (i = 0; i < MAX_CONNECTIONS; i++)
	{
		all_connections[i] = -1;
	}
	for (int i = 0; i < getNbPort(); i++)
	{
		all_connections[i] = fd[i];
	}
	while (1)
	{
		FD_ZERO(&read_fd_set);
		FD_ZERO(&write_fd_set);
		for (i = 0; i < MAX_CONNECTIONS; i++)
		{
			if (all_connections[i] >= 0)
			{
				FD_SET(all_connections[i], &read_fd_set);
			}
		}
		ret_val = select(FD_SETSIZE, &read_fd_set, &write_fd_set, NULL, NULL);
		if (ret_val >= 0)
		{
			for (int i = 0; i < getNbPort(); i++)
			{
				if (FD_ISSET(fd[i], &read_fd_set))
				{
					new_fd = accept(fd[i], (struct sockaddr *)&new_addr, &addrlen);
					fcntl(fd[i], F_SETFD, FD_CLOEXEC);
					if (new_fd >= 0)
					{
						for (i = 0; i < MAX_CONNECTIONS; i++)
						{
							if (all_connections[i] < 0)
							{
								all_connections[i] = new_fd;
								break;
							}
						}
					}
					if (new_fd == -1)
					{
						std::cout << "accept() failed for fd \n"
								  << strerror(errno) << std::endl;
					}
					ret_val--;
					if (!ret_val)
						continue;
				}
			}
			for (i = 1; i < MAX_CONNECTIONS; i++)
			{

				int err = -1;
				if ((all_connections[i] > 0) && (FD_ISSET(all_connections[i], &read_fd_set)))
				{
					err = read_connection(all_connections[i]);
					if (err == 0)
					{
						ResHandler test;
						char buf[DATA_BUFFER + 1];
						memset(buf, 0, DATA_BUFFER);
						strcpy(buf, buffu.c_str());
						test.MainServer.bando = buffu;
						std::string conttype;
						test.parse_buf(buf, test.MainServer.filename, conttype);
						ParserRequest pr(buf);
						test.MainServer.host = pr.getRequest().host;
						test.MainServer.protocol = pr.getRequest().protocol;
						test.MainServer.type = pr.getRequest().method;
						test.MainServer.path = pr.getRequest().location;
						test.MainServer.content_len = pr.getRequest().content_length;
						test.MainServer.buffit = std::string(buf);
						test.CheckModiDate();
						test.setDate();
						test.Erostatus();
						test.Methodes(FileConf);
						int checkWrit = write(all_connections[i], test.TheReposn.c_str(), (test.TheReposn.size() + 1));
						if (checkWrit == 0)
							check_probl = 0;
						else if (checkWrit == -1)
							check_probl = -1;
						close(all_connections[i]);
						fcntl(all_connections[i], F_SETFD, FD_CLOEXEC);
						FD_CLR(all_connections[i], &read_fd_set);
						buffu = "";
						test.TheReposn = "";
						all_connections[i] = -1;
						break;
					}
					if (err == 5)
					{
						close(all_connections[i]);
						all_connections[i] = -1;
					}
					if (err == -1)
					{
						break;
					}
				}
				err--;
			}
		}
	}
	close(new_fd);
	close(ret_val);
	return 0;
}
