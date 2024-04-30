#include "Utils.hpp"
#include <iostream>
#include <fstream>
#include <sstream>


namespace PolygonalLibrary {
bool ImportMesh(const string& filepath, PolygonalMesh& mesh)
{

    if(!ImportCell0Ds("Cell0Ds.csv",
                       mesh))
    {
        return false;
    }
    else
    {
        cout << "Cell0D marker:" << endl;
        for(auto it = mesh.Cell0DMarkers.begin(); it != mesh.Cell0DMarkers.end(); it++)
        {
            cout << "key:\t" << it -> first << "\t values:";
            for(const unsigned int id : it -> second)
                cout << "\t" << id;

            cout << endl;
        }
    }

    if(!ImportCell1Ds("Cell1Ds.csv",
                       mesh))
    {
        return false;
    }
    else
    {
        cout << "Cell1D marker:" << endl;
        for(auto it = mesh.Cell1DMarkers.begin(); it != mesh.Cell1DMarkers.end(); it++)
        {
            cout << "key:\t" << it -> first << "\t values:";
            for(const unsigned int id : it -> second)
                cout << "\t" << id;

            cout << endl;
        }
    }

    if(!ImportCell2Ds("Cell2Ds.csv",
                       mesh))
    {
        return false;
    }
    else
    {
        // Test:
        for(unsigned int c = 0; c < mesh.NumberCell2D; c++)
        {
            vector<unsigned int> edges = mesh.Cell2DEdges[c];
            unsigned int edges_number = edges.size();

            for(unsigned int e = 0; e < edges_number; e++)
            {
                const unsigned int origin = mesh.Cell1DVertices[edges[e]][0];
                const unsigned int end = mesh.Cell1DVertices[edges[e]][1];

                auto findOrigin = find(mesh.Cell2DVertices[c].begin(), mesh.Cell2DVertices[c].end(), origin);
                if(findOrigin == mesh.Cell2DVertices[c].end())
                {
                    cerr << "Wrong mesh" << endl;
                    return 2;
                }

                auto findEnd = find(mesh.Cell2DVertices[c].begin(), mesh.Cell2DVertices[c].end(), end);
                if(findEnd == mesh.Cell2DVertices[c].end())
                {
                    cerr << "Wrong mesh" << endl;
                    return 3;
                }

            }
        }
    }

    return true;

}
// ***************************************************************************
bool ImportCell0Ds(const string &filename,
                   PolygonalMesh& mesh)
{

    ifstream file;
    file.open(filename);

    if(file.fail())
        return false;

    list<string> listLines;
    string line;
    while (getline(file, line))
    {
        replace(line.begin(), line.end(), ';', ' ');
        listLines.push_back(line);
    }

    listLines.pop_front(); //remove the heading

    mesh.NumberCell0D = listLines.size();

    if (mesh.NumberCell0D == 0)
    {
        cerr << "There is no cell 0D" << endl;
        return false;
    }

    mesh.Cell0DId.reserve(mesh.NumberCell0D);
    mesh.Cell0DCoordinates.reserve(mesh.NumberCell0D);

    for (const string& line : listLines)
    {
        istringstream converter(line);

        unsigned int id;
        unsigned int marker;
        Vector2d coord;

        converter >>  id >> marker >> coord(0) >> coord(1);

        mesh.Cell0DId.push_back(id);
        mesh.Cell0DCoordinates.push_back(coord);

        if( marker != 0)
        {
            auto ret = mesh.Cell0DMarkers.insert({marker, {id}});
            if(!ret.second)
                (ret.first)->second.push_back(id);
        }

    }
    file.close();
    return true;
}
// ***************************************************************************
bool ImportCell1Ds(const string &filename,
                   PolygonalMesh& mesh)
{

    ifstream file;
    file.open(filename);

    if(file.fail())
        return false;

    list<string> listLines;
    string line;
    while (getline(file, line))
    {
        replace(line.begin(), line.end(), ';', ' ');
        listLines.push_back(line);
    }

    listLines.pop_front(); //remove the heading

    mesh.NumberCell1D = listLines.size();

    if (mesh.NumberCell1D == 0)
    {
        cerr << "There is no cell 1D" << endl;
        return false;
    }

    mesh.Cell1DId.reserve(mesh.NumberCell1D);
    mesh.Cell1DVertices.reserve(mesh.NumberCell1D);

    for (const string& line : listLines)
    {
        istringstream converter(line);

        unsigned int id;
        unsigned int marker;
        Vector2i vertices;

        converter >>  id >> marker >> vertices(0) >> vertices(1);

        mesh.Cell1DId.push_back(id);
        mesh.Cell1DVertices.push_back(vertices);

        if( marker != 0)
        {
            auto ret = mesh.Cell1DMarkers.insert({marker, {id}});
            if(!ret.second)
                (ret.first)->second.push_back(id);
        }
    }

    file.close();

    return true;
}
// ***************************************************************************
bool ImportCell2Ds(const string &filename,
                   PolygonalMesh& mesh)
{

    ifstream file;
    file.open(filename);

    if(file.fail())
        return false;

    list<string> listLines;
    string line;
    while (getline(file, line))
    {
        replace(line.begin(), line.end(), ';', ' ');
        listLines.push_back(line);
    }

    listLines.pop_front(); //remove the heading

    mesh.NumberCell2D = listLines.size();

    if (mesh.NumberCell2D == 0)
    {
        cerr << "There is no cell 2D" << endl;
        return false;
    }
    mesh.Cell2DId.reserve(mesh.NumberCell2D);
    mesh.Cell2DVertices.reserve(mesh.NumberCell2D);
    mesh.Cell2DEdges.reserve(mesh.NumberCell2D);

    for (const string& line : listLines)
    {
        istringstream converter(line);

        unsigned int id;
        unsigned int marker;
        unsigned int numVertices;
        unsigned int numEdges;

        converter >>  id >> marker >> numVertices; //now the number of vertices is known and I can create the vectors
        vector<unsigned int> vertices(numVertices);
        for(unsigned int i = 0; i < numVertices; i++)
            converter >> vertices[i];
        converter >> numEdges;
        vector<unsigned int> edges(numEdges);
        for(unsigned int i = 0; i < numEdges; i++)
            converter >> edges[i];

        mesh.Cell2DId.push_back(id);
        mesh.Cell2DVertices.push_back(vertices);
        mesh.Cell2DEdges.push_back(edges);
        if( marker != 0)
        {
            auto ret = mesh.Cell2DMarkers.insert({marker, {id}});
            if(!ret.second)
                (ret.first)->second.push_back(id);

        }
        file.close();

    }
    return true;
}
void CheckEdges(PolygonalMesh &mesh, double tolerance)
{
    double tol_max = max(10*numeric_limits<double>::epsilon(), tolerance); //check that the input tolerance isn't too small
    for (unsigned int e = 0; e < mesh.NumberCell1D; e++)
    {

        const unsigned int origin = mesh.Cell1DVertices[e][0];
        const unsigned int end = mesh.Cell1DVertices[e][1];

        const double originX = mesh.Cell0DCoordinates[origin][0];
        const double originY = mesh.Cell0DCoordinates[origin][1];

        const double endX = mesh.Cell0DCoordinates[end][0];
        const double endY = mesh.Cell0DCoordinates[end][1];

        double distance = sqrt(pow((originX-endX),2) + pow((originY-endY),2));
        if (distance < tol_max) {
            cout << "Edge " << e << " has a length which is smaller than tolerance " << endl;
        }

    }

    cout << "All lengths of the edges have been checked" << endl;
    return;
};
void CheckAreas(PolygonalMesh &mesh, double tolerance)
{
    double tol_max = max(10*numeric_limits<double>::epsilon(), tolerance);
    double tol2 = pow(tol_max,2);    //check that the input tolerance isn't too small

    for (unsigned int p = 0; p < mesh.NumberCell2D; p++)
    {

        vector<unsigned int> vertices = mesh.Cell2DVertices[p];
        unsigned int verticesNumber = vertices.size();

        double area_p = 0.0;

        for (unsigned int i = 0; i < verticesNumber; i++)
        {
            unsigned int currentPointIndex = vertices[i];
            double X0 = mesh.Cell0DCoordinates[currentPointIndex][0];
            double Y0 = mesh.Cell0DCoordinates[currentPointIndex][1];
            unsigned int nextPointIndex;
            if (i == verticesNumber - 1)
            {
                nextPointIndex = vertices[0];
            }
            else
            {
                nextPointIndex = vertices[i+1];
            }


            double nextX = mesh.Cell0DCoordinates[nextPointIndex][0];
            double nextY = mesh.Cell0DCoordinates[nextPointIndex][1];

            area_p += (X0*nextY - nextX*Y0);
        }

        double area = abs(area_p / 2);

        if (area < tol2)
        {
            cout << "Polygon " << p << " has area: " << area << ", which is smaller than tolerance " << endl;

        }

    }

    cout << "All areas of the polygon have been checked" << endl;
    return;

    };
}


