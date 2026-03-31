# List of identified & needed features

| feature | accepted | in dev | ready for testing | target version | available |
|------|------|------|------|-----|-----|
| handling HTTP response codes | :white_check_mark: | :white_check_mark: | :white_check_mark: | 0.1 / pre alpha | :white_check_mark: |
| handling GET/SELECT | :white_check_mark: | :white_check_mark: | :white_check_mark: | 0.1 / pre alpha | :white_check_mark: |
| status resource | :white_check_mark: | :white_check_mark: | :white_check_mark: | 0.1 / pre alpha | :white_check_mark: |
| healthcheck resource  | :white_check_mark: | :white_check_mark: | :red_square: | replaced by ping | :red_square: |
| ping resource | :white_check_mark: | :white_check_mark: | :white_check_mark: | 0.1 / pre alpha | :white_check_mark: |
| status resource extension | :white_check_mark: | :white_check_mark: | :white_large_square: | 0.2 / pre beta |:white_large_square: |
| TLS/HTTPS | :white_check_mark: | :white_check_mark: | :white_large_square: | 0.2 / pre beta |:white_large_square: |
| /auth resource | :white_check_mark: | :white_check_mark: | :white_large_square: | 0.3 / pre alpha |:white_large_square: |
| digest authentication | :white_check_mark: | :white_large_square: | :white_large_square: | 0.3 / pre alpha |:white_large_square: |
| ability to subscribe to resources | :white_check_mark: | :white_large_square: | :white_large_square: | 0.4 / pre alpha |:white_large_square: |
| linking subscription to a mrdb role | :white_check_mark: | :white_large_square: | :white_large_square: | 0.4 / pre alpha |:white_large_square: |
| default hidden schemas : mysql, rest2sql | :white_check_mark: | :white_large_square: | :white_large_square: | 0.5 / pre alpha |:white_large_square: |
| credentials + session like behaviour with libjwt-c | :white_check_mark: | :white_large_square: | :white_large_square: | 0.6 pre alpha  |:white_large_square: |
| add status variables : have_json2sql, counters | :white_check_mark: | :white_large_square: | :white_large_square: | 0.7 / alpha |:white_large_square: |
| add system variables : address, port, etc | :white_check_mark: | :white_large_square: | :white_large_square: | 0.8 / alpha | :white_large_square: |
| input sanitation | :white_check_mark: | :white_large_square: | :white_large_square: | 0.9 / beta |:white_large_square: |
| memory management hardening | :white_check_mark: | :white_large_square: | :white_large_square: | 0.9 / beta |:white_large_square: |
| release | :white_check_mark: | :white_large_square: | :white_large_square: | 1.0 |:white_large_square: |
| resource exposition black/whitelist | :white_check_mark: | :white_large_square: | :white_large_square: | 1.1 |:white_large_square: |
| handling POST/INSERT | :white_check_mark: | :white_large_square: | :white_large_square: | 1.2 |:white_large_square: |
| handling PATCH/UPDATE | :white_check_mark: | :white_large_square: | :white_large_square: | 1.3 | :white_large_square: |
| handling DELETE/DELETE | :white_check_mark: | :white_check_mark: | :white_large_square: | 1.4  |:white_large_square: |
| handling UPSERTS | :grey_question: | :grey_question: | :grey_question: | :grey_question: | :grey_question: |
| handling simple sort order | :grey_question: | :grey_question: | :grey_question: | :grey_question: | :grey_question: | :grey_question: |
| handling simple joins for select | :red_square: | | | NO (CRUD) | :red_square: |
| handling PUT/CALL SP | :red_square: | | | NO (CRUD) | :red_square: |
| handling DDL | :red_square: | | | NO (CRUD) | :red_square: |
| handling DCL | :red_square: | | | NO (security) | :red_square: |
| handling TCL | :red_square: | | | NO (CRUD) | :red_square: |
| resource discovery  | :red_square: |  |  | NO (security) |  :red_square: |
| using global/local internal connexion | :red_square: | | | NO (security) | :red_square: |
| handling mutli-lines INSERTs | :red_square: |  |  | NO (CRUD) | :red_square: |

