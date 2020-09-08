#include <iostream>

#include <cassert>
#include <errno.h>
#include <cstdio>
#include <cstdlib>
#include <systemd/sd-lldp.h>
#include <sdbus-c++/sdbus-c++.h>

/*
g++ -I/home/tomas/zdrojaky/cesnet/systemd/build/install/usr/include /home/tomas/zdrojaky/cesnet/systemd/build/install/usr/lib/systemd/libsystemd-shared-246.so -lsdbus-c++ test.c && LD_LIBRARY_PATH=/home/tomas/zdrojaky/cesnet/systemd/build/install/usr/lib/systemd ./a.out
*/

static int open_lldp_neighbors(int ifindex, FILE **ret) {
        char* p = NULL;
        FILE *f;

        if (asprintf(&p, "/run/systemd/netif/lldp/%i", ifindex) < 0)
                return -ENOMEM;

        f = fopen(p, "re");
        free ( p );
        if (!f)
                return -errno;

        *ret = f;
        return 0;
}

static int next_lldp_neighbor(FILE *f, sd_lldp_neighbor **ret) {
        void *raw = NULL;
        size_t l;
        size_t u;
        int r;

        assert(f);
        assert(ret);

        l = fread(&u, 1, sizeof(u), f);
        if (l == 0 && feof(f))
                return 0;
        if (l != sizeof(u))
                return -EBADMSG;

        /* each LLDP packet is at most MTU size, but let's allow up to 4KiB just in case */
        if (le64toh(u) >= 4096)
                return -EBADMSG;

        raw = malloc(sizeof(uint8_t) * le64toh(u));
        if (!raw)
                return -ENOMEM;

        if (fread(raw, 1, le64toh(u), f) != le64toh(u))
                return -EBADMSG;

        r = sd_lldp_neighbor_from_raw(ret, raw, le64toh(u));
        free ( raw );
        if (r < 0)
                return r;

        return 1;
}

std::vector<sdbus::Struct<int, std::string, sdbus::ObjectPath>> list_links() {
	std::vector<sdbus::Struct<int, std::string, sdbus::ObjectPath>> links;
	auto managerObj = sdbus::createProxy("org.freedesktop.network1", "/org/freedesktop/network1");
	managerObj->callMethod("ListLinks").onInterface("org.freedesktop.network1.Manager").storeResultsTo(links);

	for ( const auto & link : links ) {
		std::cerr << "Link " << std::get<0>(link) << " " << std::get<1>(link) << " " << std::get<2>(link) << std::endl;
	}

	return links;
}


int main ( ) {
	for ( const auto & link : list_links() ) {
		FILE *f;
		sd_lldp_neighbor *n;

		if ( open_lldp_neighbors(std::get<0>(link), &f) < 0 )
			break;

		for ( ;; ) {
			if ( next_lldp_neighbor(f, &n) <= 0 )
				break;

			const char *system_name = NULL, *port_id = NULL, *port_description = NULL;
			(void) sd_lldp_neighbor_get_system_name(n, &system_name);
			(void) sd_lldp_neighbor_get_port_id_as_string(n, &port_id);
			(void) sd_lldp_neighbor_get_port_description(n, &port_description);

			printf("LLDP: %s: %s | %s | %s\n", std::get<1>(link).c_str(), system_name, port_id, port_description );
			sd_lldp_neighbor_unrefp ( &n );
		}

		fclose(f);
	}
}
