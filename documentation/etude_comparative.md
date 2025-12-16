# Étude Technique et Comparative - Projet R-Type

## 1. Gestion des Dépendances : CMake FetchContent

**Choix retenu :** CMake FetchContent

**Justification :**
FetchContent permet d'intégrer directement les dépendances dans le build sans installation système.

**Comparaison :**

| Critère | FetchContent | Conan | Vcpkg |
|---------|--------------|-------|-------|
| Intégration CMake | Native | Nécessite configuration | Nécessite toolchain |
| Complexité initiale | Minimale | Élevée | Moyenne |
| Portabilité | Excellente | Bonne | Bonne |

**Avantages :** Simplicité d'utilisation, pas d'outil externe requis, contrôle total des versions via Git.

**Inconvénients :** Temps de compilation initial plus long, gestion manuelle des versions.

---

## 2. Bibliothèque Réseau : Sockets Natifs Encapsulés

**Choix retenu :** BSD Sockets (Linux) / Winsock (Windows) avec encapsulation objet

**Architecture :**
- **Classe Address :** encapsule `sockaddr_in`
- **Classe NetworkSocket :** encapsule les opérations socket
- **Classes Server/Client :** logique métier réseau

**Comparaison avec Asio :**

| Critère | Sockets Natifs | Asio/Boost.Asio |
|---------|----------------|-----------------|
| Contrôle | Total | Abstrait |
| Dépendances | Aucune | Boost (~100MB) |
| Performance | Optimale | Très bonne |
| Apprentissage | Mécanismes bas niveau | API haut niveau |

**Justification :**
- Compréhension approfondie des mécanismes réseau
- Aucune dépendance lourde
- Performance optimale sans couche d'abstraction supplémentaire
- Flexibilité totale pour notre protocole binaire

---

## 3. Bibliothèque Graphique : SFML

**Choix retenu :** Simple and Fast Multimedia Library

**Comparaison :**

| Critère | SFML | SDL2 | Raylib |
|---------|------|------|--------|
| Facilité (2D) | Très simple | Moyenne | Très simple |
| Audio intégré | Oui | Module séparé | Oui |
| Communauté | Large | Très large | Moyenne |

**Justification :**
- Expérience préalable de l'équipe avec SFML
- Parfaitement adaptée à un jeu 2D (sprites, textures, animations)
- Module audio intégré pour les effets sonores

---

## 4. Architecture Moteur : Entity-Component-System (ECS)

**Choix retenu :** ECS

**Principe :**
- **Entity :** Simple ID (index)
- **Component :** Données pures (Position, Velocity, Sprite, Health...)
- **System :** Logique (MovementSystem, CollisionSystem...)

**Comparaison :**

| Approche | Découplage | Flexibilité | Performance | Complexité |
|----------|-----------|-------------|-------------|------------|
| **ECS** | Très élevé | Très élevée | Excellente | Moyenne |
| POO | Faible | Faible | Moyenne | Faible |
| Data-Driven pur | Élevé | Élevée | Excellente | Élevée |

**Justification :**
- **Découplage maximal :** Les systèmes sont indépendants
- **Composition flexible :** Facile de créer de nouvelles entités en combinant des composants
- **Performance :** Composants contigus en mémoire (cache-friendly)
- **Extensibilité :** Ajouter un comportement = nouveau Component + System
- **Réutilisabilité :** Les systèmes fonctionnent pour toutes les entités ayant les composants requis

L'ECS évite les problèmes de hiérarchies d'héritage complexes et facilite l'ajout de nouveaux comportements sans modifier le code existant.

---

## 5. Protocole Réseau : UDP Pur

**Choix retenu :** UDP exclusivement

**Comparaison UDP vs TCP :**

| Critère | UDP | TCP |
|---------|-----|-----|
| Latence | Très faible | Plus élevée |
| Fiabilité | Non garantie | Garantie |
| Use case gaming | Actions temps-réel | Données critiques |

**Justification :**
- **Latence minimale** essentielle pour un jeu multijoueur
- Pas de head-of-line blocking (paquets indépendants)
- Acceptation de la perte de paquets (positions mises à jour 60x/sec)

**Évolution possible :** Notre architecture `NetworkSocket` supporte TCP en option pour des fonctionnalités futures (chat, connexion initiale, messages critiques).

---

## 6. Structures de Données : Registry + SparseArray

**Choix retenu :** Registry contenant des SparseArray de composants

**Architecture :**
- **SparseArray\<T\> :** `std::vector<std::optional<Component>>`
- **Entity :** Simple `size_t` (index)
- Les entités sont des indices dans tous les SparseArrays

**Comparaison :**

| Structure | Localité mémoire | Complexité |
|-----------|------------------|------------|
| **SparseArray** | Excellente | Faible |
| HashMap | Mauvaise | Moyenne |

**Justification :**
- **Cache-friendly :** Composants du même type à côté dans la mémoire
- **Simplicité :** `std::optional` gère élégamment les composants absents
- **Performance :** Itération rapide sur les composants pour les systèmes
- **Flexibilité :** Ajout/suppression de composants triviale

---

## Conclusion

Les choix techniques privilégient :
- **Simplicité :** Outils familiers (SFML, CMake FetchContent)
- **Contrôle :** Sockets natifs pour comprendre les mécanismes réseau
- **Performance :** ECS cache-friendly, UDP pour faible latence
- **Extensibilité :** Architecture découplée, protocole flexible
