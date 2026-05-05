#include "header.h"
using namespace std;
int main()
{
	Accounts* accounts = new Accounts;
	Admin admin;
	admin.sortedPayments(accounts);
	admin.findRevenue(accounts);

}