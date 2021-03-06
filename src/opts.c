#include <libconfig.h>

#include "opts.h"
#include "util.h"

static void parse_cli_opts(struct opts *opts, int argc, char **argv) {
    while (argc-- > 1) {
        char *s = argv[argc];
        while (*s) {
            switch (*(s++)) {
            case '-':
                break;
            case 'u':
                opts->_skip_fetch = 1;
                break;
            case 'g':
                opts->_skip_generate = 1;
                break;
            case 'k':
                opts->_skip_hook = 1;
                break;
            default:
                panic("USAGE: %s [-ugk]\n"
                      "See /usr/share/doc/pacwall/README.rst for more info.",
                      argv[0]);
            }
        }
    }
}

static void config_lookup_escape(config_t *cfg, const char *path, const char **out) {
    const char *str = NULL;
    config_lookup_string(cfg, path, &str);
    if (str == NULL) {
        return;
    }

    char *tmp = strdup(str);
    for (char *cp = tmp; *cp; ++cp) {
        if (*cp == '"') {
            *cp = '\'';
        } else if (*cp == '\'') {
            *cp = '"';
        }
    }

    *out = tmp;
}

struct opts parse_opts(int argc, char **argv) {
    /*INDENT-OFF*/
    struct opts opts = {
        .hook = NULL,
        .shell = "bash",
        .db = "/var/lib/pacman",
        .attributes_graph = "bgcolor=\"#00000000\"",
        .attributes_package_common = "shape=point, height=0.1,"
        "fontname=monospace, fontsize=10",
        .attributes_package_implicit = "color=\"#dc322faa\"",
        .attributes_package_explicit = "color=\"#268bd2aa\"",
        .attributes_package_orphan = "color=\"#2aa198aa\", peripheries=2,"
                                     "fontcolor=\"#2aa198\", xlabel=\"\\N\",",
        .attributes_package_outdated = "color=\"#b58900aa\", peripheries=3,"
                                       "fontcolor=\"#b58900\", xlabel=\"\\N\"",
        .attributes_dependency_common = "color=\"#fdf6e311\"",
        .attributes_dependency_hard = "",
        .attributes_dependency_optional = "arrowhead=empty, style=dashed",
        ._skip_fetch = 0,
        ._skip_generate = 0,
        ._skip_hook = 0
    };
    /*INDENT-ON*/

    config_t cfg;
    config_init(&cfg);

    chdir_xdg("XDG_CONFIG_HOME", ".config/", "pacwall");
    FILE *cfg_file = fopen("pacwall.conf", "r");
    if (cfg_file == NULL) {
        panic("Could not open pacwall.conf: %s\n"
              "Refer to /usr/share/doc/pacwall/README.rst for a configuration guide",
              strerror(errno));
    }
    if (!config_read(&cfg, cfg_file)) {
        panic("Malformed pacwall.conf (line %d): %s",
              config_error_line(&cfg),
              config_error_text(&cfg));
    }
    fclose(cfg_file);

    config_lookup_escape(&cfg, "hook", &opts.hook);
    config_lookup_escape(&cfg, "shell", &opts.shell);
    config_lookup_escape(&cfg, "db", &opts.db);
    config_lookup_escape(&cfg, "attributes.graph", &opts.attributes_graph);
    config_lookup_escape(&cfg, "attributes.package.common",
                         &opts.attributes_package_common);
    config_lookup_escape(&cfg, "attributes.package.implicit",
                         &opts.attributes_package_implicit);
    config_lookup_escape(&cfg, "attributes.package.explicit",
                         &opts.attributes_package_explicit);
    config_lookup_escape(&cfg, "attributes.package.orphan",
                         &opts.attributes_package_orphan);
    config_lookup_escape(&cfg, "attributes.package.outdated",
                         &opts.attributes_package_outdated);
    config_lookup_escape(&cfg, "attributes.dependency.common",
                         &opts.attributes_dependency_common);
    config_lookup_escape(&cfg, "attributes.dependency.hard",
                         &opts.attributes_dependency_hard);
    config_lookup_escape(&cfg, "attributes.dependency.optional",
                         &opts.attributes_dependency_optional);
    config_destroy(&cfg);

    parse_cli_opts(&opts, argc, argv);

    return opts;
}
