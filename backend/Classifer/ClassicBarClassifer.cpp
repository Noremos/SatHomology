// module;
#include "ClassicBarClassifer.h"


GlobalClassRegister<BarlineClass, BaritemHolder, barclassificator> a("barlines");
GlobalClassRegister<BettylineClass, BettyItemHolder, BettyClassificator> b("betty");
//MEXPORT int fyBettyClasssifier = GlobalRegister<BettylineClass, BettyItemHolder, BettyClassificator>().getId();
