/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fejjed <fejjed@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/30 10:57:05 by tamighi           #+#    #+#             */
/*   Updated: 2022/07/05 11:23:46 by tamighi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "server/Server.hpp"
#include "config/ParserConfig.hpp"

int main(int b , char **argv)
{
	if(b ==2)
	{
    try 
    {
        ParserConfig    cf(argv[1]);
        std::vector<ServerMembers> servers = cf.getConfig();

		Server serv(servers);		
		std::string FileConf;
		FileConf = argv[1];
        serv.run(FileConf);
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
	}
	return 0;
}
