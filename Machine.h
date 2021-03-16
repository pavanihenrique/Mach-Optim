#ifndef MACHINE
#define MACHINE

class Machine {
private:
	unsigned long long avail, price, resell, gain;

public:
	Machine(unsigned long long D = 0, unsigned long long P = 0, unsigned long long R = 0, unsigned long long G = 0) :avail{ D }, price{ P }, resell{ R }, gain{ G } {};
	~Machine() {};
	unsigned long long getAvailability() const { return avail; };
	unsigned long long getPrice() const { return price; };
	unsigned long long getResellPrice() const { return resell; };
	unsigned long long getGain() const { return gain; };
};

#endif // !MACHINE

