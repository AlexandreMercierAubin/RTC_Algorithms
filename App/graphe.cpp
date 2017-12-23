//
//  Graphe.cpp
//  Classe pour graphes orientés pondérés (non négativement) avec listes d'adjacence
//
//  Mario Marchand automne 2016.
//

#include "graphe.h"

using namespace std;

//! \brief Constructeur avec paramètre du nombre de sommets désiré
//! \param[in] p_nbSommets indique le nombre de sommets désiré
//! \post crée le vecteur de p_nbSommets de listes d'adjacence vides
Graphe::Graphe(size_t p_nbSommets)
    : m_listesAdj(p_nbSommets), nbArcs(0)
{
}

//! \brief change le nombre de sommets du graphe
//! \param[in] p_nouvelleTaille indique le nouveau nombre de sommet
//! \post le graphe est un vecteur de p_nouvelleTaille de listes d'adjacence
//! \post les anciennes listes d'adjacence sont toujours présentes lorsque p_nouvelleTaille >= à l'ancienne taille
//! \post les dernières listes d'adjacence sont enlevées lorsque p_nouvelleTaille < à l'ancienne taille
void Graphe::resize(size_t p_nouvelleTaille)
{
    m_listesAdj.resize(p_nouvelleTaille);
}

size_t Graphe::getNbSommets() const
{
	return m_listesAdj.size();
}

size_t Graphe::getNbArcs() const
{
    return nbArcs;
}

//! \brief ajoute un arc d'un poids donné dans le graphe
//! \param[in] i: le sommet origine de l'arc
//! \param[in] j: le sommet destination de l'arc
//! \param[in] poids: le poids de l'arc
//! \pre les sommets i et j doivent exister
//! \throws logic_error lorsque le sommet i ou le sommet j n'existe pas
//! \throws logic_error lorsque le poids == numeric_limits<unsigned int>::max()
void Graphe::ajouterArc(size_t i, size_t j, unsigned int poids)
{
    if (i >= m_listesAdj.size())
        throw logic_error("Graphe::ajouterArc(): tentative d'ajouter l'arc(i,j) avec un sommet i inexistant");
    if (j >= m_listesAdj.size())
        throw logic_error("Graphe::ajouterArc(): tentative d'ajouter l'arc(i,j) avec un sommet j inexistant");
    if (poids == numeric_limits<unsigned int>::max())
        throw logic_error("Graphe::ajouterArc(): valeur de poids interdite");
    m_listesAdj[i].emplace_back(Arc(j, poids));
    ++nbArcs;
}

//! \brief enlève un arc dans le graphe
//! \param[in] i: le sommet origine de l'arc
//! \param[in] j: le sommet destination de l'arc
//! \pre l'arc (i,j) et les sommets i et j dovent exister
//! \post enlève l'arc mais n'enlève jamais le sommet i
//! \throws logic_error lorsque le sommet i ou le sommet j n'existe pas
//! \throws logic_error lorsque l'arc n'existe pas
void Graphe::enleverArc(size_t i, size_t j)
{
    if (i >= m_listesAdj.size())
        throw logic_error("Graphe::enleverArc(): tentative d'enlever l'arc(i,j) avec un sommet i inexistant");
    if (j >= m_listesAdj.size())
        throw logic_error("Graphe::enleverArc(): tentative d'enlever l'arc(i,j) avec un sommet j inexistant");
    auto &liste = m_listesAdj[i];
    bool arc_enleve = false;
    for (auto itr = liste.end(); itr != liste.begin();) //on débute par la fin par choix
    {
        if ((--itr)->destination == j)
        {
            liste.erase(itr);
            arc_enleve = true;
            break;
        }
    }
    if (!arc_enleve)
        throw logic_error("Graphe::enleverArc: cet arc n'existe pas; donc impossible de l'enlever");
    --nbArcs;
}


unsigned int Graphe::getPoids(size_t i, size_t j) const
{
    if (i >= m_listesAdj.size()) throw logic_error("Graphe::getPoids(): l'incice i n,est pas un sommet existant");
    for (auto & arc : m_listesAdj[i])
    {
        if (arc.destination == j) return arc.poids;
    }
    throw logic_error("Graphe::getPoids(): l'arc(i,j) est inexistant");
}


void  Graphe::triTopologique(size_t sommet, vector<bool> &visite, stack<size_t> &tri) const
{
    //marquer comme visite
    visite[sommet]=true;

    for (auto u_itr = m_listesAdj[sommet].begin(); u_itr != m_listesAdj[sommet].end(); ++u_itr)
    {
        if (!visite[u_itr->destination])
        {
            triTopologique(u_itr->destination,visite,tri);
        }
    }
    tri.push(sommet);
}


//! \brief Algorithme de BellmanFord pour les graphes acycliques.
//! \pre p_origine et p_destination doivent être des sommets du graphe
//! \return la longueur du plus court chemin est retournée
//! \param[out] le chemin est retourné (un seul noeud si p_destination == p_origine ou si p_destination est inatteignable)
//! \return la longueur du chemin (= numeric_limits<unsigned int>::max() si p_destination n'est pas atteignable)
//! \throws logic_error lorsque p_origine ou p_destination n'existe pas
unsigned int Graphe::pccBellmanFord(size_t p_origine, size_t p_destination, std::vector<size_t> &p_chemin) const
{
    try {

        p_chemin.clear();

        if (p_origine == p_destination)
        {
            p_chemin.push_back(p_destination);
            return 0;
        }
        vector<unsigned int> distance(m_listesAdj.size(), numeric_limits<unsigned int>::max());
        vector<size_t> predecesseur(m_listesAdj.size(), numeric_limits<size_t>::max());
        vector<bool> listeFerme(m_listesAdj.size(), false);

        distance[p_origine] = 0;

        stack<size_t> tri;
        for(size_t i=0;i<m_listesAdj.size();++i)
        {
            if(!listeFerme[i])
            {
                triTopologique(i,listeFerme,tri);
            }
        }



        //Boucle principale: touver distance[] et predecesseur[]
        while (!tri.empty()) {
            size_t sommet = tri.top(); //ramasser le noeud en traitement
            tri.pop(); //enlever le noeud de listeOuvert

            if (distance[sommet]!=numeric_limits<unsigned int>::max()) {
                //relâcher les arcs
                for (auto u_itr = m_listesAdj[sommet].begin(); u_itr != m_listesAdj[sommet].end(); ++u_itr) {


                    //chercher la nouvelle distance
                    unsigned int nouvelleDistance = distance[sommet] + u_itr->poids;


                    if (nouvelleDistance < distance[u_itr->destination]) {
                        distance[u_itr->destination] = nouvelleDistance;
                        predecesseur[u_itr->destination] = sommet;
                    }
                }
            }

        }

        //cas où l'on n'a pas de solution
        if (predecesseur[p_destination] == numeric_limits<unsigned int>::max())
        {
            p_chemin.push_back(p_destination);
            return numeric_limits<unsigned int>::max();
        }

        //On a une solution, donc construire le plus court chemin à l'aide de predecesseur[]
        stack<size_t> pileDuChemin;
        size_t numero = p_destination;
        pileDuChemin.push(numero);
        while (predecesseur[numero] != numeric_limits<size_t>::max())
        {
            numero = predecesseur[numero];
            pileDuChemin.push(numero);
        }
        while (!pileDuChemin.empty())
        {
            size_t temp = pileDuChemin.top();
            p_chemin.push_back(temp);
            pileDuChemin.pop();
        }
        return distance[p_destination];
    }catch(...)
    {
       throw logic_error("Une erreur est survenue");
    }
}


//! \brief Algorithme de Dijkstra permettant de trouver le plus court chemin entre p_origine et p_destination
//! \pre p_origine et p_destination doivent être des sommets du graphe
//! \return la longueur du plus court chemin est retournée
//! \param[out] le chemin est retourné (un seul noeud si p_destination == p_origine ou si p_destination est inatteignable)
//! \return la longueur du chemin (= numeric_limits<unsigned int>::max() si p_destination n'est pas atteignable)
//! \throws logic_error lorsque p_origine ou p_destination n'existe pas
unsigned int Graphe::plusCourtChemin(size_t p_origine, size_t p_destination, std::vector<size_t> &p_chemin) const
{
    try {
        if (p_origine >= m_listesAdj.size() || p_destination >= m_listesAdj.size())
            throw logic_error("Graphe::dijkstra(): p_origine ou p_destination n'existe pas");

        p_chemin.clear();

        if (p_origine == p_destination)
        {
            p_chemin.push_back(p_destination);
            return 0;
        }
        vector<unsigned int> distance(m_listesAdj.size(), numeric_limits<unsigned int>::max());
        vector<size_t> predecesseur(m_listesAdj.size(), numeric_limits<size_t>::max());
        distance[p_origine] = 0;


        vector<size_t> listeOuvert = {p_origine}; //ensemble des noeuds non solutionnés
        make_heap(listeOuvert.begin(), listeOuvert.end()); //faire un tas avec le vecteur



        //Boucle principale: touver distance[] et predecesseur[]
        while (!listeOuvert.empty()) {
            size_t sommet = listeOuvert.back(); //ramasser le noeud en traitement
            listeOuvert.pop_back(); //enlever le noeud de listeOuvert

            //relâcher les arcs
            for (auto u_itr = m_listesAdj[sommet].begin(); u_itr != m_listesAdj[sommet].end(); ++u_itr)
            {
                if (u_itr->poids < 0) //gestion d'erreur, car l'algorithme n'accepte pas les negatifs
                    throw logic_error("arc de poids negatif");

                //chercher la nouvelle distance
                unsigned int nouvelleDistance = distance[sommet] + u_itr->poids;


                if (nouvelleDistance < distance[u_itr->destination])
                {
                    distance[u_itr->destination] = nouvelleDistance;
                    predecesseur[u_itr->destination] = sommet;
                    listeOuvert.push_back(u_itr->destination);
                    push_heap(listeOuvert.begin(), listeOuvert.end());
                }
            }

        }

        //cas où l'on n'a pas de solution
        if (predecesseur[p_destination] == numeric_limits<unsigned int>::max())
        {
            p_chemin.push_back(p_destination);
            return numeric_limits<unsigned int>::max();
        }

        //On a une solution, donc construire le plus court chemin à l'aide de predecesseur[]
        stack<size_t> pileDuChemin;
        size_t numero = p_destination;
        pileDuChemin.push(numero);
        while (predecesseur[numero] != numeric_limits<size_t>::max())
        {
            numero = predecesseur[numero];
            pileDuChemin.push(numero);
        }
        while (!pileDuChemin.empty())
        {
            size_t temp = pileDuChemin.top();
            p_chemin.push_back(temp);
            pileDuChemin.pop();
        }
        return distance[p_destination];
    }catch(...)
    {
        throw logic_error("Une erreur est survenue");
    }
}

/*ancienne version du plus court chemin pour les tests de performances*/
unsigned int Graphe::legacyplusCourtChemin(size_t p_origine, size_t p_destination, std::vector<size_t> &p_chemin) const
{
    if (p_origine >= m_listesAdj.size() || p_destination >= m_listesAdj.size())
        throw logic_error("Graphe::dijkstra(): p_origine ou p_destination n'existe pas");

    p_chemin.clear();

    if (p_origine == p_destination)
    {
        p_chemin.push_back(p_destination);
        return 0;
    }
    vector<unsigned int> distance(m_listesAdj.size(), numeric_limits<unsigned int>::max());
    vector<size_t> predecesseur(m_listesAdj.size(), numeric_limits<size_t>::max());
    distance[p_origine] = 0;
    //ajouter fgh ici au besoin

    list<size_t> q; //ensemble des noeuds non solutionnés;
    for (size_t i = 0; i < m_listesAdj.size(); ++i) //construction de q
    {
        q.push_back(i);
    }

    //Boucle principale: touver distance[] et predecesseur[]
    while (!q.empty())
    {
        //trouver uStar dans q tel que distance[uStar] est minimal
        list<size_t>::iterator uStar_itr = q.end();
        unsigned int min = numeric_limits<unsigned int>::max();
        for (auto itr = q.begin(); itr != q.end(); ++itr)
        {
            if (distance[*itr] < min)
            {
                min = distance[*itr];
                uStar_itr = itr;
            }
        }
        if (uStar_itr == q.end()) break; //quitter la boucle : il est impossible de se rendre à destination

        size_t uStar = *uStar_itr; //le noeud solutionné
        q.erase(uStar_itr); //l'enlevé de q

        if (uStar == p_destination) break; //car on a obtenu distance[p_destination] et predecesseur[p_destination]

        //relâcher les arcs sortant de uStar
        for (auto u_itr = m_listesAdj[uStar].begin(); u_itr != m_listesAdj[uStar].end(); ++u_itr)
        {
            unsigned int temp = distance[uStar] + u_itr->poids;
            if (temp < distance[u_itr->destination])
            {
                distance[u_itr->destination] = temp;
                predecesseur[u_itr->destination] = uStar;
            }
        }
    }

    //cas où l'on n'a pas de solution
    if (predecesseur[p_destination] == numeric_limits<unsigned int>::max())
    {
        p_chemin.push_back(p_destination);
        return numeric_limits<unsigned int>::max();
    }

    //On a une solution, donc construire le plus court chemin à l'aide de predecesseur[]
    stack<size_t> pileDuChemin;
    size_t numero = p_destination;
    pileDuChemin.push(numero);
    while (predecesseur[numero] != numeric_limits<size_t>::max())
    {
        numero = predecesseur[numero];
        pileDuChemin.push(numero);
    }
    while (!pileDuChemin.empty())
    {
        size_t temp = pileDuChemin.top();
        p_chemin.push_back(temp);
        pileDuChemin.pop();
    }
    return distance[p_destination];
}
