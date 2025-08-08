# List of identified & needed features
| feature | accepted | in dev | ready for testing | target version | 
|------|------|------|------|-----|
| handling HTTP response codes | :white_check_mark: | :white_check_mark: | :white_check_mark: | 0.1 / pre alpha |
| handling GET/SELECT | :white_check_mark: | :white_check_mark: | :white_check_mark: | 0.1 / pre alpha |
| status resource | :white_check_mark: | :white_check_mark: | :white_check_mark: | 0.1 / pre alpha |
| healthcheck resource  | :white_check_mark: | :white_check_mark: | :red_square: | replaced by ping |
| ping resource | :white_check_mark: | :white_check_mark: | :white_check_mark: | 0.1 / pre alpha |
| handling DELETE/DELETE | :white_check_mark: | :white_check_mark: | :white_large_square: | 0.2 / pre alpha |
| status resource extension | :white_check_mark: | :white_check_mark: | :white_large_square: | 0.2 / pre alpha |
| securing the API with HTTPS | :white_check_mark: | :white_large_square: | :white_large_square: | 0.3 / pre alpha |
| ability to subscribe to resources | :white_check_mark: | :white_large_square: | :white_large_square: | 0.4 / pre alpha |
| linking subscription to a mrdb role | :white_check_mark: | :white_large_square: | :white_large_square: | 0.4 / pre alpha |
| handling credentials with JWT (lib to be choosen) | :grey_question: | :white_large_square: | :white_large_square: | 0.5 pre alpha  |
| handling POST/INSERT | :white_check_mark: | :white_large_square: | :white_large_square: | 0.6 / pre alpha |
| handling PATCH/UPDATE | :white_check_mark: | :white_large_square: | :white_large_square: | 0.6 / pre alpha | 
| add status variables : have_json2sql, counters | :white_check_mark: | :white_large_square: | :white_large_square: | 0.7 / alpha |
| add system variables : address, port, resource exposition list | :white_check_mark: | :white_large_square: | :white_large_square: | 0.8 / alpha | 
| input sanitation | :white_check_mark: | :white_large_square: | :white_large_square: | beta |
| memory management hardening | :white_check_mark: | :white_large_square: | :white_large_square: | beta |
| handling UPSERTS | :grey_question: | :grey_question: | :grey_question: | :grey_question: |
| handling simple sort order | :grey_question: | :grey_question: | :grey_question: | :grey_question: | :grey_question: |
| handling PUT/CALL stored procedure | :grey_question: | :grey_question: | :grey_question: |  :grey_question: | :grey_question:
| handling DDL | :red_square: | || not CRUD |
| handling DCL | :red_square: | || no administrative task through CRUD REST |
| handling TCL | :red_square: | || no trx in CRUD |
| resource discovery  | :red_square: |  |  | unwise for security | 
| handling simple joins for select | :red_square: |  || CRUD has no joins |
| using global/local internal connexion | :red_square: | | | not allowing RBAC |
| handling mutli-lines INSERTs | :red_square: |  |  | too complicated |

* No order of importance here.
