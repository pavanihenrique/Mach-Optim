#include <fstream>
#include <string> 
#include <iostream>
#include <stdlib.h>
#include <list>
#include <map>
#include <memory>
#include "Machine.h"

using MachineMap = std::map<int, std::shared_ptr<Machine>>;
using MachineList = std::list<Machine *>;
//using MachineList = std::list<std::shared_ptr<Machine>>;

void populate_dict(std::ifstream& input, MachineMap& dict, int N, int T) {
	int D = 0, P = 0, R = 0, G = 0;
	int i = 0, key = 1;
	//Machine* obj;
	//Machine* aux;
	MachineList aux_list;
	
	for (i = 0; i < N; i++) {
		input >> D >> P >> R >> G;
		aux_list.push_back(new Machine(D, P, R, G));
	}

	for (i = 1; i <= T; i++) {
		for (MachineList::iterator it = aux_list.begin(); it != aux_list.end(); ++it) {
			if ((*it)->getAvailability() == i) {
				std::shared_ptr<Machine> p(*it);
				dict.insert(make_pair(key, p));
				++key;
			}
		}
	}

/*	for (std::list<Machine *>::iterator it = dict.begin(); it != dict.end(); ++it) {
		if (it == dict.begin()) ++it;
		else {
			value = (*it)->getAvailability();
			obj = (*it);
			std::list<Machine*>::iterator j = it;
			--j;

			while (j != dict.begin() && (*j)->getAvailability() > value)
			{
				aux = (*j);
				++j;
				(*j) = aux;
				--j;
				--j;
			}
			++j;
			(*j) = obj;
		}
	}
*/
	return;
}

int check_availability(MachineMap& dict, int curr_day) {
	int found = 0;

	for (auto it = dict.begin(); it != dict.end(); ++it) {
		if ((*(it->second)).getAvailability() == curr_day)	found = 1;
	}

	return found;
}

int buy_decision(MachineMap& dict, int curr_machine, int curr_money, int curr_day, int T) {
	int result = 0, curr_projection = 0, candidate_projection = 0, breakeven = 0;
	
	int days_left = T - curr_day;
	int curr_result = curr_money;
	int avail_money = curr_money;

	int best_candidate = curr_machine;
	int best_result = curr_result;
	int best_breakeven = curr_day;

	if (curr_machine != 0) {
		curr_result = curr_result + curr_money + dict[best_candidate]->getGain() * (days_left + 1) + dict[best_candidate]->getResellPrice();
		avail_money = avail_money + dict[curr_machine]->getResellPrice();
	}

	// encontra o melhor candidato de compra baseado no resultado final, se mantido, e breakeven (critério de desempate)
	for (auto& it : dict) {
		if ((*(it.second)).getAvailability() == curr_day) {
			if (avail_money >= (*(it.second)).getPrice()) {
				result = avail_money - (*(it.second)).getPrice() + (*(it.second)).getGain() * days_left + (*(it.second)).getResellPrice();

				breakeven = curr_day + 1;
				curr_projection = curr_money;
				if (curr_machine != 0) curr_projection = curr_projection + dict[curr_machine]->getGain();
				candidate_projection = avail_money - (*(it.second)).getPrice();
				while (curr_projection > candidate_projection && breakeven < T) {
					++breakeven;
					if (curr_machine != 0) curr_projection = curr_projection + dict[curr_machine]->getGain();
					candidate_projection = candidate_projection + (*(it.second)).getGain();
				}

				if (result > best_result || (best_candidate != curr_machine && result == best_result && breakeven < best_breakeven)) {
					best_candidate = it.first;
					best_result = result;
					best_breakeven = breakeven;
				}
			}
		}
		else if ((*(it.second)).getAvailability() > curr_day) break;
	}

	//verifica se existe uma melhor opção de compra durante o período de breakeven do melhor candidato
	if (best_candidate != curr_machine && best_breakeven > curr_day + 2) {
		curr_projection = curr_money;
		avail_money = curr_money;
		for (int day = curr_day + 1; day <= best_breakeven; day++) {
			if (curr_machine != 0) {
				curr_projection = curr_projection + dict[curr_machine]->getGain() * (day - curr_day);
				avail_money = curr_projection + dict[curr_machine]->getResellPrice();
			}
			days_left = T - day;
			for (auto& it : dict) {
				if ((*(it.second)).getAvailability() == day) {
					if (avail_money >= (*(it.second)).getPrice()) {
						result = avail_money - (*(it.second)).getPrice() + (*(it.second)).getGain() * days_left + (*(it.second)).getResellPrice();

						if (result > best_result) {
							best_candidate = 0;
							break;
						}
					}
				}
				else if ((*(it.second)).getAvailability() > day) break;
			}
		}
	}

	if (best_candidate == curr_machine) best_candidate = 0;
	
	return best_candidate;
}

int optim_solver(int N, int M, int T, MachineMap& dict) {
	int curr_machine = 0, curr_money = M;
	int buy = 0;
	//int D = 0, P = 0, R = 0, G = 0;

	for (int curr_day = 1; curr_day <= T; ++curr_day) {
		if (check_availability(dict, curr_day)) {
			buy = buy_decision(dict, curr_machine, curr_money, curr_day, T);
			if (buy) {
				if (curr_machine != 0) curr_money = curr_money + dict[curr_machine]->getResellPrice();
				curr_machine = buy;
				curr_money = curr_money - dict[curr_machine]->getPrice();
			}
			else if (curr_machine != 0) curr_money = curr_money + dict[curr_machine]->getGain();
		}
		else if (curr_machine != 0) curr_money = curr_money + dict[curr_machine]->getGain();
	}

	if (curr_machine != 0) curr_money = curr_money + dict[curr_machine]->getResellPrice();

	return curr_money;
}

int main () {
	int nbr_machines = 0, i_money = 0, t_days = 0;
	//int curr_machine = 0, curr_money = 0, curr_day = 0;
	//int D = 0, P = 0, R = 0, G = 0;
	int end_run = 0, case_nbr = 0, f_money = 0;
	//std::list<Machine*> MachineList;
	MachineMap MachineDict;

	std::ifstream input("input.txt");

	while (end_run == 0) {
		case_nbr++;
		input >> nbr_machines >> i_money >> t_days;
		if (nbr_machines == 0 && i_money == 0 && t_days == 0) {
			end_run = 1;
			std::cout << "End of file." << std::endl;
		}
		else if (i_money == 0 || t_days <= 1) std::cout << "Case " << case_nbr << ": " << i_money << std::endl;
		else if (nbr_machines != 0) {
			populate_dict(input, MachineDict, nbr_machines, t_days);
			f_money = optim_solver(nbr_machines, i_money, t_days, MachineDict);
			std::cout << "Case " << case_nbr << ": " << f_money << std::endl;
		}
		else std::cout << "Case " << case_nbr << ": " << i_money << std::endl;
	}
}