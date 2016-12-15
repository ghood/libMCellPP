//
// THIS IS STILL UNDER CONSTRUCTION AND VERY UNPOLISHED!!!
//

#include "mcell.hh"
using namespace MCell;

// 1. Unbounded diffusion

class Molecule {
public:
  Molecule ();
  virtual double diffusionRate () { return(0.0); }
  virtual double random ();   // uniform over [0..1)
  MoleculeID id;
  Point position;
private:
  RNGState rngState;
}

class A: public Molecule ()
{
  double diffusionRate () { return (10.0); }
};


main ()
{
  RegisterClass(A);

  Simulation sim;
  //  sim.wrap(XAxis, -1000.0, 1000);
  sim.release(1000, "A", Sphere(Origin, 100.0));
  sim.run(1000.0);
}

// 2. "1D diffusion" in a thin tube
//      * initial release in a plane in the middle

class A: public Molecule ()
{
  double diffusionRate () { return (10.0); }
};

enum {
  Reflective, Absorptive, Adsorptive
} SurfaceType;

class Surface ()
{
  SurfaceType insideDefault () { return Reflective; }
  SurfaceType outsideDefault () { return Reflective; }
}

main ()
{
  RegisterClass(A);

  Simulation sim;
  sim.wrap(XAxis, -1000.0, 1000);
  Region tube = Cylinder(Origin, XAxis, 100.0);    // 100 = radius in um
  sim.surface("Surface", tube);
  sim.release(1000, "A", Intersection(tube, Plane(Origin, XAxis)));
  sim.run(1000.0);
}

// 2D diffusion" between two sheets
// initial release in a line source
class A: public Molecule ()
{
  double diffusionRate () { return (10.0); }
};

main ()
{
  RegisterClass(A);

  Simulation sim;
  sim.wrap(XAxis, -1000.0, 1000.0);
  sim.wrap(YAxis, -1000.0, 1000.0);
  sim.surface("ReflectiveSurface", Plane(Point(0.0, 0.0, -1.0), -ZAxis));
  sim.surface("ReflectiveSurface", Plane(Point(0.0, 0.0, 1.0), ZAxis));
  sim.release(1000, "A", LineSegment(Point(-1000.0, 0.0, 0.0),
				     Point(1000.0, 0.0, 0.0),
				     false, true));
  sim.run(1000.0);
}


// 4. 2D diffusion on a real 2D surface
// * initial release on a patch by either density or Boolean intersection
//   of e.g. a sphere with a plane
class A: public Molecule ()
{
  double diffusionRate () { return (10.0); }
};

main ()
{
  RegisterClass(A);

  Simulation sim;
  sim.wrap(XAxis, -1000.0, 1000.0);
  SurfaceID surface = sim.surface("AdsorptiveSurface", Plane(Origin, ZAxis));
  sim.releaseOnSurface(1000, "A", surface, Front,
		       Intersection(Sphere(Origin, 100.0),
				    surface));
  sim.run(1000.0);
}


// 5. Volumetric diffusion in a box/sphere
main ()
{
  RegisterClass(A);

  Simulation sim;
  sim.surface("ReflectiveSurface",
	      Box(Point(-500.0, -500.0, -500.0),
		  Point(500.0, 500.0, 500.0)));
  sim.release("A", 1000,
	      Box(Point(-500.0, -500.0, -500.0),
		  Point(500.0, 500.0, 500.0)));

  sim.surface("ReflectiveSurface",
	      Sphere(Origin, 500.0));
  sim.release("A", 1000,
	      Sphere(Origin, 500.0));
  sim.run(1000.0);
}


// 6. Box of A & B with reactions:
//  * A -> 0 decay
class A: public Molecule ()
{
  double diffusionRate () { return (10.0); }
  run ()
    {
      schedule(computeDecayTime(), Decay);
    }
};

main ()
{
  RegisterClass(A);

  Simulation sim;
  sim.surface("ReflectiveSurface",
	      Box(Point(-500.0, -500.0, -500.0),
		  Point(500.0, 500.0, 500.0)));
  sim.release("A", 1000,
	      Box(Point(-500.0, -500.0, -500.0),
		  Point(500.0, 500.0, 500.0)));
  sim.run(1000.0);
}

//  * A -> B decay
class A: public Molecule ()
{
  double diffusionRate () { return (10.0); }
  run (Simulation& sim)
    {
      schedule(computeDecayTime(), Decay);
    }
  decay (Simulation& sim)
    {
      sim.release("B", At(position));
    }
};

class B: public Molecule ()
{
  double diffusionRate () { return (20.0); }
};


//  * A + B -> C irreversible
class A: public Molecule ()
{
  double diffusionRate () { return (10.0); }
  A (Simulation& sim)
    {
      sim.triggerOnDistance("B", distance, (Result)(*react)(), this->id);
    }
  Result react (Simulation& sim, Molecule& m)
    {
      A& a = *this;
      B& b = dynamic_cast<B&>(m);
      sim.remove(a.id);
      sim.remove(b.id);
      sim.release("C", At(Average(a.position, b.position)));
    }
};
class B: public Molecule ()
{
  double diffusionRate () { return (20.0); }
  // no need to set up distance trigger because A will do that;
  //   assumption is that A's are less numerous than B's
};
class C: public Molecule ()
{
  double diffusionRate () { return (10.0); }
}

//  * A + B <-> C reversible

class A: public Molecule ()
{
  double diffusionRate () { return (10.0); }
  A (Simulation& sim)
    {
      sim.triggerOnDistance("B", distance, (Result)(*react)(), this->id);
    }
  Result react (Simulation& sim, Molecule& m)
    {
      A& a = *this;
      B& b = dynamic_cast<B&>(m);
      sim.remove(a.id);
      sim.remove(b.id);
      sim.release("C", At(Average(a.position, b.position)));
    }
};
class B: public Molecule ()
{
  double diffusionRate () { return (10.0); }
  // no need to set up distance trigger because A will do that;
  //   assumption is that A's are less numerous than B's
};
class C: public Molecule ()
{
  double diffusionRate () { return (10.0); }
  C (Simulation& sim)
    {
      sim.triggerOnTime(computeDecayTime(), (Result)(*decay)(), this->id);
    }
  Result decay (Simulation& sim)
    {
      sim.release("A", At(position));
      sim.release("B", At(position));
    }
};

// 7. Box of A & B with reactions on a surface:

//  * A (volumetric) + B (surface) -> C (surface)


// 8. Surface classes
// * Box with one transparent side to molecule A
// * Box with absorptive side to molecule A
// * Box with concentration clamp of A on one side and
//   absorptive on the opposing side


// 9. Object with a generally complex geometry from some list of vertices and faces
vector<Point> vertices;   // coordinates of all vertices
vector<int> vertexListIndices;  // indices into vertices array; -1 terminates a list;
                                //   list is always in clockwise order as viewed from
                                //   the outside
vector<int> faces;        // starting index into vertexLists for each face
sim.surface("Surface", Mesh(vertices, vertexListIndices, faces));

// 10. Dynamic changes
//     * Dynamic geometry: an interface to change the mesh at each timestep
//     * Example: An initial box that updates it's geometry at each timestep
//       with some new list of vertices and faces
class Surface {
  Surface ()
  {
    t(event, t+deltaT);
  };

  update (Time t);
};
mesh = Mesh(vertices, vertexLists, faces);
s = sim.surface("Surface", mesh);
vector<int> modifyVertices, modifyVertexListIndices, modifyFaces;
vector<Point> modifiedVertices;
vector<int> modifiedVertexListIndices, modifiedFaces;
m.modify(newVertices, newVertexLists, newFaces);
m.partialModify(modifyVertices, modifiedVertices,
		modifyVertexListIndices, modifiedVertexListIndices,
		modifyFaces, modifiedFaces);


//     * Dynamic rate constants: changing the rate constant of a reaction
//     * Example: A box with A+B->C that where the rate constant depends on the number of A,B, or C


//     * Dynamic diffusion constants: changing the diffusion constant for a molecule
//     * Example: A box of diffusing A particles that increase their diffusion constants over time
class A: public Molecule ()
{
  double diffusionRate (Time t) { return (some_function(t)); }
};

// 11. Counting statements
//     * A box (maybe with some region definition) with A+B->C reaction where we count each of the following at each timestep:
//     * What: molecule, reaction, event/trigger
//     * Where: World, object, region
//     * When: frequency of counting
//     * How: Front hits/back hits, e.g. on a plane that goes through the box


// 12. SubClassing Molecule Species
//     * Implement a subclass of molecule that occupies space

// 13. SubClassing Molecules and Reactions
//     * Implement subclasses that use BioNetGen-like molecules and reactions

// 14. SubClassing Diffusion
//     * Implement a subclass that uses a different kind of diffusion

// 15. SubClassing the Scheduler
//     * Implement a subclass that uses a variable time-step scheduler
