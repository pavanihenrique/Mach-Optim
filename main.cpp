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

void populate_dict(std::ifstream& input, MachineMap& dict, int N, int T) {
	int D = 0, P = 0, R = 0, G = 0;
	int i = 0, key = 1, min = 0;
	MachineList aux_list;	
	MachineList::iterator it_min = aux_list.begin();

	for (i = 0; i < N; i++) {
		input >> D >> P >> R >> G;
		aux_list.push_back(new Machine(D, P, R, G));
	}

	for (i = 0; i < N; ++i) {
		it_min = aux_list.begin();
		min = (*it_min)->getAvailability();
		for (MachineList::iterator it = aux_list.begin(); it != aux_list.end(); ++it) {
			if ((*it)->getAvailability() < min) {
				it_min = it;
				min = (*it)->getAvailability();
			}
		}
		std::shared_ptr<Machine> p(*it_min);
		dict.insert(make_pair(key, p));
		++key;
		aux_list.erase(it_min);
	}

	return;
}

void reset_dict(MachineMap& dict) {
	for (MachineMap::iterator it = dict.begin(); it != dict.end(); ++it) {
		(it->second).reset();
	}
	MachineMap::iterator dict_begin = dict.begin();
	MachineMap::iterator dict_end = dict.end();
	dict.erase(dict_begin, dict_end);
}

int check_availability(MachineMap& dict, int curr_day) {
	for (auto it = dict.begin(); it != dict.end(); ++it) {
		if ((*(it->second)).getAvailability() == curr_day)	return it->first;
	}

	return 0;
}

int buy_decision(MachineMap& dict, int curr_machine, int curr_money, int curr_day, int T, int i_index) {
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

	for (MachineMap::iterator it = dict.find(i_index); it != dict.end(); ++it) {
		if ((*(it->second)).getAvailability() == curr_day) {
			if (avail_money >= (*(it->second)).getPrice()) {
				result = avail_money - (*(it->second)).getPrice() + (*(it->second)).getGain() * days_left + (*(it->second)).getResellPrice();

				breakeven = curr_day + 1;
				int a = 0, b = 0, q = 0;
				if (curr_machine != 0) a = (*(it->second)).getPrice() - dict[curr_machine]->getResellPrice();
				else a = (*(it->second)).getPrice();
				if (curr_machine != 0) b = (*(it->second)).getGain() - dict[curr_machine]->getGain();
				else b = (*(it->second)).getGain();
				if (b == 0) continue;
				else breakeven = breakeven + a / b + (a % b != 0);

				if (result > best_result || (best_candidate != curr_machine && result == best_result && breakeven < best_breakeven)) {
					best_candidate = it->first;
					best_result = result;
					best_breakeven = breakeven;
				}
			}
		}
		else if ((*(it->second)).getAvailability() > curr_day) break;
	}

	if (best_candidate != curr_machine && best_breakeven > curr_day + 2) {
		curr_projection = curr_money;
		avail_money = curr_money;
		for (MachineMap::iterator it = dict.find(i_index); it != dict.end(); ++it) {
			if (curr_machine != 0) {
				curr_projection = curr_projection + dict[curr_machine]->getGain() * ((*(it->second)).getAvailability() - curr_day);
				avail_money = curr_projection + dict[curr_machine]->getResellPrice();
			}
			if ((*(it->second)).getAvailability() >= best_breakeven) break;
			if (avail_money >= (*(it->second)).getPrice()) {
				days_left = T - (*(it->second)).getAvailability();
				result = avail_money - (*(it->second)).getPrice() + (*(it->second)).getGain() * days_left + (*(it->second)).getResellPrice();
				if (result > best_result) return 0;
			}
			
		}
	}

	if (best_candidate == curr_machine) best_candidate = 0;
	
	return best_candidate;
}

int optim_solver(int N, int M, int T, MachineMap& dict) {
	int curr_machine = 0, curr_money = M, curr_day = 1, next_dec = 0, next_index = 1;
	int avail_index = 0, buy = 0;

	MachineMap::iterator it = dict.begin();
	while (it != dict.end()) {
		next_dec = (it->second)->getAvailability();
		if (curr_machine != 0) curr_money = curr_money + (dict[curr_machine]->getGain()) * (next_dec - curr_day - 1);
		curr_day = next_dec;
		buy = buy_decision(dict, curr_machine, curr_money, curr_day, T, it->first);
		if (buy) {
			if (curr_machine != 0) curr_money = curr_money + dict[curr_machine]->getResellPrice();
			curr_machine = buy;
			if (curr_machine != 0) curr_money = curr_money - dict[curr_machine]->getPrice();
		}
		else if (curr_machine != 0) curr_money = curr_money + dict[curr_machine]->getGain();
		while ((it->second)->getAvailability() == curr_day) {
			++it;
			if (it == dict.end()) break;
		}
	}

	if (curr_machine != 0) curr_money = curr_money + (dict[curr_machine]->getGain()) * (T - curr_day);

	if (curr_machine != 0) curr_money = curr_money + dict[curr_machine]->getResellPrice();

	return curr_money;
}

int main () {
	int nbr_machines = 0, i_money = 0, t_days = 0;
	int end_run = 0, case_nbr = 0, f_money = 0;
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
			reset_dict(MachineDict);
		}
		else std::cout << "Case " << case_nbr << ": " << i_money << std::endl;
	}
}