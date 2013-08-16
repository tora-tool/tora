#include <stdlib.h>

#include <coptypes.h>       /* Teradata include files */
#include <coperr.h>
#include <dbcarea.h>
#include <parcel.h>
#include <dbchqep.h>

int main(int c, char **v) {
    static Int32  result;
    static char   cnta[4];
    struct DBCAREA *dbcp;

    dbcp = (DBCAREA *) malloc(sizeof(struct DBCAREA));
    dbcp->total_len = sizeof(struct DBCAREA);
    DBCHINI(&result,cnta, dbcp);

    return 0;
}
