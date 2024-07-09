#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <limits.h>
#include <time.h>
#include <pthread.h>				


//pts indique les graduations, dist garde en mémoire les distances entre les différents points
typedef struct rule {int* pts;
					 int size;
					 bool* dist;} Rule;//règle construite à l'envers


Rule create_rule (int n, int max){
	//initialisation de la règle
	int* arr_pts = malloc(n*sizeof(int));
	bool* arr_dist = malloc(max*sizeof(bool));
	for (int i = 0; i < max; i++){
		arr_dist[i] = false;
	}
	Rule regle = {.pts = arr_pts, .size = n, .dist = arr_dist};
	return regle;}


void clear_rule (Rule r){
	//libérer la règle
	free(r.pts);
	free(r.dist);
}

//On applique un algorithme de backtracking pour calculer la règle optimale

void retire_pt (Rule* ptr_r, int pt, int j){
	//supprime le dernier point de la règle, i.e. le point j tel que pour i > j, la position d'indice i de la règle n'est pas encore renseignée, et tel que pt est la graduation d'indice j
	//assert(j < ptr_r->size);
	for (int k = 0; k<j; k++){
		int d = ptr_r->pts[k] - pt;
		ptr_r->dist[d] = false;
	}
}

bool is_gr_ajout (Rule* ptr_r, int i, int pt, int* ptr_j){ //test en O(n)
	//regle est un règle de Golomb, on teste si en ajoutant pt elle reste de Golomb. si oui on modifie en place regle en y ajoutant le nouveau point
	bool possible = true;
	int j = 0;
	while (possible && j < i){
		int d = ptr_r->pts[j] - pt;
		if (!ptr_r->dist[d]){ptr_r->dist[d] = true;}
		else {possible = false;};
		j++;
	}
	if (possible) *ptr_j = j;
	else *ptr_j = j - 1;
	ptr_r->pts[i] = pt;
	return possible;}

/*
int minimum(int n1, int n2){
	if (n1 < n2) return n1;
	return n2;
}

int maximum(int n1, int n2){
	if (n1 > n2) return n1;
	return n2;
}*/

bool ogr(Rule* regle, int* mins, int i, int inf, int sup){
	//renvoie si la règle telle que les i premières cases sont déjà remplies peut être complétée en une règle de golomb et modifie en place la règle pour qu'elle contienne une règle de Golomb optimale
	//mins correspond à un tableau qui contient, pour tout k < regle->size, la taille d'une règle de golomb minimale d'ordre k
	if (i == regle->size){return true;}
	else {
		bool found = false;
		for (int pt = inf; !found && pt < sup; pt ++){
			int j;
			bool res = is_gr_ajout(regle, i, pt, &j);
			if (res && ogr(regle, mins, i + 1, mins[regle->size - i - 1], pt)) found = true;
			else retire_pt(regle, pt, j);
		}
		return found;
	}
}


void print_regle(Rule regle){
	printf("OGR d'ordre %d: ",regle.size);
	for (int i = 0; i < regle.size; i++){
		printf("| %d |", regle.pts[regle.size - 1 - i]);
	}
	printf("\n");
}

struct arguments{
	Rule* regle;
	bool* trouve;
	int* mins;
	int inf;
	int sup;
};

typedef struct arguments args;

static void * ogr_aux(void * argus){
	args* a = (args*) argus;
	bool b = ogr(a->regle, a->mins, 1, a->inf, a->sup);
	*(a->trouve) = b;
	return NULL;
}
	

Rule regle_thread(int k, int* mins, int max){
	Rule regle1 = create_rule(k, max);
	Rule regle2 = create_rule(k, max);
	bool found1 = false;
	bool found2 = false;
	
	pthread_t fil1;
	pthread_t fil2;
	cpu_set_t cpuset1;
	cpu_set_t cpuset2;
	CPU_ZERO(&cpuset1);
	CPU_ZERO(&cpuset2);
	for (size_t j = 0; j < 4; j++) {
		CPU_SET(j, &cpuset1);
		CPU_SET(j + 4, &cpuset2);
	}
	pthread_setaffinity_np(fil1, sizeof(cpuset1), &cpuset1);
	pthread_setaffinity_np(fil1, sizeof(cpuset2), &cpuset2);
	
	for (int i = mins[k - 1] + 2; !found1 && !found2; i++){
		regle1.pts[0] = i;
		regle2.pts[0] = i;
		int m = (mins[k-1] + i)/2;
		args a1 = {.regle = &regle1, .mins = mins, .trouve = &found1, .inf = mins[k-1], .sup = m};
		args a2 = {.regle = &regle2, .mins = mins, .trouve = &found2, .inf = m, .sup = i};
		pthread_create(&fil1, NULL, ogr_aux, (void *) &a1);
		pthread_create(&fil2, NULL, ogr_aux, (void *) &a2);
		pthread_join(fil1, NULL);
		pthread_join(fil2, NULL);
	}
	if (found1) return regle1;
	return regle2;
}
		

void print_ogr(int n){
	assert(n>=3);
	int* mins = malloc(n*sizeof(int));
	//mins[0] = 0;
	mins[1] = 0; // règle d'ordre 1 : |0|
	mins[2] = 1; // règle d'ordre 2 : |0|1|
	printf("OGR d'ordre 1: | 0 |\nOGR d'ordre 2: | 0 || 1 |\n");
	clock_t start = clock ();
	int max = 2*mins[2] + 1;
	for (int k = 3; k<n; k++){
		Rule regle = create_rule (k, max);
		ogr(&regle, mins, 0, mins[k-1] + 2, 1<<(k-1));
		print_regle(regle);
		//printf("taille du tableau: %d par rapport à %d \n\n", max, 1<<(k - 1));
		mins[k] = regle.pts[0];
		max = 2*regle.pts[0] + 1; //on peut créer une règle de golomb en ajoutant une garduation à 2*graduation_maximale + 1
		clear_rule(regle);
	}
	Rule regle = regle_thread(n, mins, max);
	clock_t end = clock();
	double temps = ((double) (end - start))/CLOCKS_PER_SEC;
	printf("temps nécessaire pour l'ordre %d : %f s \n", n, temps);
	print_regle(regle);
	//printf("taille du tableau: %d\n par rapport à %d \n", max, 1<<(n - 1));
	clear_rule(regle);
	free(mins);
}


int main(void){
	print_ogr(11);
	return 0;}
