//Atividade realizada por: Warley Fernandes Xavier da Silva

#include <stdio.h>
#include <stack>
#include <stdlib.h>

#include <vector>

#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>


std::mutex mtx; 



std::mutex print_mtx;
std::mutex maze_mtx;
std::atomic<bool> exit_found(false);


// Matriz de char representnado o labirinto
char** maze; // Voce também pode representar o labirinto como um vetor de vetores de char (vector<vector<char>>)

// Numero de linhas e colunas do labirinto
int num_rows;
int num_cols;

// Representação de uma posição
struct pos_t {
	int i;
	int j;
};

// Estrutura de dados contendo as próximas
// posicões a serem exploradas no labirinto
std::stack<pos_t> valid_positions;
/* Inserir elemento: 

	 pos_t pos;
	 pos.i = 1;
	 pos.j = 3;
	 valid_positions.push(pos)
 */
// Retornar o numero de elementos: 
//    valid_positions.size();
// 
// Retornar o elemento no topo: 
//  valid_positions.top(); 
// 
// Remover o primeiro elemento do vetor: 
//    valid_positions.pop();


//std::vector<std::thread> thread_vec;

int n_active_threads = 0;


// Função que le o labirinto de um arquivo texto, carrega em 
// memória e retorna a posição inicial
pos_t load_maze(const char* file_name) {
	pos_t initial_pos;
	// Abre o arquivo para leitura (fopen)

	FILE *file = fopen(file_name, "r");

	if (file == NULL) {
		initial_pos.i = -1;
		initial_pos.j = -1;
		return initial_pos;
	}

	// Le o numero de linhas e colunas (fscanf) 
	// e salva em num_rows e num_cols
	fscanf(file, "%d %d", &num_rows, &num_cols);


	// Aloca a matriz maze (malloc)
	maze = (char**)malloc(num_rows * sizeof(char*));
	for (int i = 0; i < num_rows; ++i)
		// Aloca cada linha da matriz
		maze[i] = (char*)malloc(num_cols * sizeof(char)); 
	
	for (int i = 0; i < num_rows; ++i) {
		for (int j = 0; j < num_cols; ++j) {
			// Le o valor da linha i+1,j do arquivo e salva na posição maze[i][j]
			char ch = fgetc(file);
            while (ch == '\n') {
                ch = fgetc(file);
            }
            maze[i][j] = ch;
			// Se o valor for 'e' salvar o valor em initial_pos
            if (ch == 'e') {
                initial_pos.i = i;
                initial_pos.j = j;
            }
		}
	}

	fclose(file);

	return initial_pos;
}

// Função que imprime o labirinto
void print_maze() {
	for (int i = 0; i < num_rows; ++i) {
		for (int j = 0; j < num_cols; ++j) {
			printf("%c", maze[i][j]);
		}
		printf("\n");
	}
}



// Verifica se a posição é válida e não visitada
bool isValid(int i, int j) {
    return i >= 0 && i < num_rows && j >= 0 && j < num_cols && (maze[i][j] == 'x' || maze[i][j] == 's');
}


// Função responsável pela navegação.
// Recebe como entrada a posição initial e retorna um booleando indicando se a saída foi encontrada
bool walk(pos_t pos) {
    if (exit_found) return false;

    if (maze[pos.i][pos.j] == 's') {
        exit_found = true;
        return true;
    }

    std::vector<pos_t> local_stack;
    local_stack.push_back(pos);

    while (!local_stack.empty() && !exit_found) {
        pos_t current_pos = local_stack.back();
        local_stack.pop_back();

        maze_mtx.lock();
        maze[current_pos.i][current_pos.j] = 'o';
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
		maze[current_pos.i][current_pos.j] = '.';
        maze_mtx.unlock();
        print_mtx.lock();
        print_mtx.unlock();

        pos_t next_positions[4] = {
            {current_pos.i, current_pos.j + 1}, 
            {current_pos.i, current_pos.j - 1}, 
            {current_pos.i + 1, current_pos.j}, 
            {current_pos.i - 1, current_pos.j}
        };

        for (auto& next_pos : next_positions) {
            if (isValid(next_pos.i, next_pos.j)) {
				maze[current_pos.i][current_pos.j] = '.';
                if (maze[next_pos.i][next_pos.j] == 's') {
                    exit_found = true;
                    return true;
                }
                local_stack.push_back(next_pos);
            }
        }
    }

    return false;
}

void explore(pos_t start_pos) {
    walk(start_pos);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}


bool sholdTerminate = false;


void print_matriz(){
	while (!sholdTerminate)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		system("clear");
		print_maze();
	}
	system("clear");
	print_maze();
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
}




int main(int argc, char* argv[]) {
	// carregar o labirinto com o nome do arquivo recebido como argumento
	pos_t initial_pos = load_maze(argv[1]);
	// chamar a função de navegação

	std::thread t(print_matriz);

	std::thread main_thread(explore, initial_pos); // Inicia a exploração em uma nova thread

    main_thread.join();

	sholdTerminate = true;

	t.join();

	// Tratar o retorno (imprimir mensagem)
	if (exit_found)
	{
		printf("\nSAIDA ENCONTRADA COM SUCESSO");
	} else {
		printf("NAO FOI POSSIVEL ENCONTRAR UMA SAIDA");
	}

	return 0;
}
