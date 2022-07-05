/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParserConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fejjed <fejjed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/03 14:28:49 by tamighi           #+#    #+#             */
/*   Updated: 2022/06/27 13:55:11 by fejjed           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSERCONFIG_HPP
# define PARSERCONFIG_HPP

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include "../utils/Utils.hpp"

struct ConfigMembers
{
	ConfigMembers(void)
		: root(""), max_body_size(1000000000), autoindex(false)	
	{
	}

	std::string							root;
	size_t								max_body_size;
	std::map<int, std::string>			error_pages;
	bool								autoindex;
	std::vector<std::string>			index;
	std::map<std::string, std::string>	cgis;
};

struct	LocationMembers;
struct ServerMembers : public ConfigMembers
{
	ServerMembers(void)
		: ConfigMembers(), host("0.0.0.0"), port(80)
	{
	}

	ServerMembers(const ServerMembers& cpy);

	std::string							host;
	int									port;
	std::vector<LocationMembers>		locations;
	std::vector<std::string>			server_name;

	std::map<std::string, std::string>	cgi_paths;
	std::string							upload;
	std::string							redirect;
};

struct LocationMembers : public ConfigMembers
{
	LocationMembers(void)
	{
	}

	LocationMembers(const ServerMembers& cpy);

	LocationMembers(const LocationMembers& cpy);

	std::string					uri;
	std::set<std::string>		allowedMethods;
};

class ParserConfig
{

public:
	//	Public member functions
	ParserConfig(const std::string& file_name);
	int autoindx[65535];
	~ParserConfig(void);

	std::vector<ServerMembers>&	getConfig(void);

private:
	//	Private member functions

	//	Main parse functions
	void	parse(void);
	void	parseMainCtx(std::string& line);
	void	parseServerCtx(std::string& line);
	void	parseLocationCtx(std::string& line);

	//	Secondary parse functions
	void	endScope(void);

	//	Shared functions for server and localisation
	void	addRoot(std::stringstream& ss, ConfigMembers& cm);
	void	addMaxBodySize(std::stringstream& ss, ConfigMembers& cm);
	void	addErrorPages(std::stringstream& ss, ConfigMembers& cm);
	void	addIndex(std::stringstream& ss, ConfigMembers& cm);
	void	addAutoIndex(std::stringstream& ss, ConfigMembers& cm);
	void	addCgis(std::stringstream& ss, ConfigMembers& cm);

	//	Server functions
	void	addListen(std::stringstream& ss, ServerMembers& cm);
	void	addLocation(std::stringstream& ss, ServerMembers& sm);
	void	addServerName(std::stringstream& ss, ServerMembers& sm);
	void	addRedirect(std::stringstream& ss, ServerMembers& sm);
	void	addCgiPaths(std::stringstream& ss, ServerMembers& sm);
	void	addUpload(std::stringstream& ss, ServerMembers& sm);

	//	Location functions
	void	addAllowedMethods(std::stringstream& ss, LocationMembers& lm);

	void	addDefaultServerValues(void);

	//	Private classes/structs
	enum Context
	{
		MAIN,
		SERVER,
		LOCATION
	};

	//	Private members
	std::ifstream	m_ifs;
	Context			m_ctx;
	size_t			m_curr_line;

	std::vector<ServerMembers>	m_cms;
};

std::ostream&	operator<<(std::ostream &ostr, ParserConfig& pc);

#endif
