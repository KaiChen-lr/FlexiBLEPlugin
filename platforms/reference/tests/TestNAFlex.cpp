#include "OpenMM.h"
#include "FlexiBLEForce.h"
#include "FlexiBLEKernels.h"
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <fstream>
#include "PosVec.h"
using namespace std;
using namespace FlexiBLE;
using namespace OpenMM;

extern "C" OPENMM_EXPORT void registerFlexiBLEReferenceKernelFactories();

void writePdbFrame(int frameNum, const State &state, string fileName)
{
    // Reference atomic positions in the OpenMM State.
    const vector<Vec3> &posInNm = state.getPositions();
    fstream fout(fileName, ios::app);
    // Use PDB MODEL cards to number trajectory frames
    // printf("MODEL     %d\n", frameNum); // start of frame
    fout << "MODEL     " << frameNum << endl;
    for (int a = 0; a < (int)(posInNm.size() / 2); ++a)
    {
        // printf("ATOM  %5d  AR   AR     1    ", a + 1); // atom number
        fout << "ATOM  " << setw(5) << a + 1 << "  NE   NE     1    ";
        // printf("%8.3f%8.3f%8.3f  1.00  0.00\n",        // coordinates
        fout << setw(8) << fixed << setprecision(3) << posInNm[a][0] * 10;
        fout << setw(8) << fixed << setprecision(3) << posInNm[a][1] * 10;
        fout << setw(8) << fixed << setprecision(3) << posInNm[a][2] * 10 << endl;
        //                                                // "*10" converts nanometers to Angstroms
        //        posInNm[a][0] * 10, posInNm[a][1] * 10, posInNm[a][2] * 10);
    }
    for (int a = (int)(posInNm.size() / 2); a < (int)posInNm.size(); ++a)
    {
        // printf("ATOM  %5d  AR   AR     1    ", a + 1); // atom number
        fout << "ATOM  " << setw(5) << a + 1 << "  AR   AR     2    ";
        // printf("%8.3f%8.3f%8.3f  1.00  0.00\n",        // coordinates
        fout << setw(8) << fixed << setprecision(3) << posInNm[a][0] * 10;
        fout << setw(8) << fixed << setprecision(3) << posInNm[a][1] * 10;
        fout << setw(8) << fixed << setprecision(3) << posInNm[a][2] * 10 << endl;
        //                                                // "*10" converts nanometers to Angstroms
        //        posInNm[a][0] * 10, posInNm[a][1] * 10, posInNm[a][2] * 10);
    }
    fout.unsetf(ios::fixed);
    // printf("ENDMDL\n"); // end of frame
    fout << "ENDMDL" << endl;
}

void writeVelocites(int frameNum, const State &state, string fileName)
{
    // Reference atomic positions in the OpenMM State.
    const vector<Vec3> &velInNm = state.getVelocities();
    fstream fout2(fileName, ios::app);
    // Use PDB MODEL cards to number trajectory frames
    // printf("MODEL     %d\n", frameNum); // start of frame
    fout2 << "MODEL     " << frameNum << endl;
    for (int a = 0; a < (int)(velInNm.size() / 2); ++a)
    {
        // printf("ATOM  %5d  AR   AR     1    ", a + 1); // atom number
        fout2 << "ATOM  " << setw(5) << a + 1 << "  NE   NE     1    ";
        // printf("%8.3f%8.3f%8.3f  1.00  0.00\n",        // coordinates
        fout2 << setw(8) << fixed << setprecision(3) << velInNm[a][0] * 10;
        fout2 << setw(8) << fixed << setprecision(3) << velInNm[a][1] * 10;
        fout2 << setw(8) << fixed << setprecision(3) << velInNm[a][2] * 10 << endl;
        // "*10" converts nanometers to Angstroms
        //        posInNm[a][0] * 10, posInNm[a][1] * 10, posInNm[a][2] * 10);
    }
    for (int a = velInNm.size() / 2; a < (int)velInNm.size(); ++a)
    {
        // printf("ATOM  %5d  AR   AR     1    ", a + 1); // atom number
        fout2 << "ATOM  " << setw(5) << a + 1 << "  AR   AR     1    ";
        // printf("%8.3f%8.3f%8.3f  1.00  0.00\n",        // coordinates
        fout2 << setw(8) << fixed << setprecision(3) << velInNm[a][0] * 10;
        fout2 << setw(8) << fixed << setprecision(3) << velInNm[a][1] * 10;
        fout2 << setw(8) << fixed << setprecision(3) << velInNm[a][2] * 10 << endl;
        // "*10" converts nanometers to Angstroms
        //        posInNm[a][0] * 10, posInNm[a][1] * 10, posInNm[a][2] * 10);
    }
    fout2.unsetf(ios::fixed);
    // printf("ENDMDL\n"); // end of frame
    fout2 << "ENDMDL" << endl;
}

void simulateNeon()
{
    OpenMM::Platform::loadPluginsFromDirectory(
        OpenMM::Platform::getDefaultPluginsDirectory());
    fstream data_out("NA_Flex.txt", ios::out);
    // Create a system with nonbonded forces.
    System system;
    NonbondedForce *nonbond = new NonbondedForce();
    system.addForce(nonbond);
    CustomExternalForce *exforce = new CustomExternalForce("100*max(0, r-1.55)^2; r=sqrt(x*x+y*y+z*z)");
    FlexiBLEForce *boundary = new FlexiBLEForce();
    vector<int> InputQMIndices = {0, 3, 14, 33, 52, 53, 65, 68, 83, 89, 117, 136, 143, 164, 165, 166, 182, 186, 189, 197};
    vector<int> InputMLInfo = {100, 1, 100, 1};
    vector<int> AssignedIndex = {-1, -1};
    vector<double> InputThre = {1e-5, 1e-5};
    vector<int> InputMaxIt = {10, 10};
    vector<double> InputScales = {0.5, 0.5};
    vector<double> InputAlphas = {50.0, 50.0};
    vector<vector<double>> Centers = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
    vector<vector<double>> Capsules = {{0.2, 0.0, 0.0}, {0.2, 0.0, 0.0}};
    boundary->SetQMIndices(InputQMIndices);
    boundary->SetMoleculeInfo(InputMLInfo);
    boundary->SetAssignedIndex(AssignedIndex);
    boundary->GroupingMolecules();
    boundary->SetInitialThre(InputThre);
    boundary->SetFlexiBLEMaxIt(InputMaxIt);
    boundary->SetScales(InputScales);
    boundary->SetAlphas(InputAlphas);
    boundary->SetBoundaryType(2, Capsules);
    boundary->SetTestOutput(1);
    boundary->SetValOutput(1);
    boundary->SetTemperature(163.0);
    system.addForce(boundary);
    system.addForce(exforce);
    //  Create three atoms.
    vector<Vec3> initPosInNm(200);
    vector<Vec3> initVelocities(200);
    for (int a = 0; a < 200; a++)
    {
        initPosInNm[a] = Vec3(NAPositions[a][0], NAPositions[a][1], NAPositions[a][2]); // location, nm
        initVelocities[a] = Vec3(NAVelocities[a][0], NAVelocities[a][1], NAVelocities[a][2]);
        if (a == 0)
            system.addParticle(0.0); // mass of Neon, grams per mole
        else if (a > 0 && a < 100)
            system.addParticle(20.1797);
        else if (a >= 100)
            system.addParticle(39.95);
        // charge, L-J sigma (nm), well depth (kJ)
        if (a < 100)
            nonbond->addParticle(0.0, 0.2782, 0.298); // vdWRad(Ar)=.188 nm
        else if (a >= 100)
            nonbond->addParticle(0.0, 0.34, 1.0036);
        exforce->addParticle(a, vector<double>());
    }

    // LangevinIntegrator integrator(163, 1, 0.001); // step size in ps
    VerletIntegrator integrator(0.004);
    // Let OpenMM Context choose best platform.
    Platform &platform = Platform::getPlatformByName("Reference");
    Context context(system, integrator, platform);
    // printf("REMARK  Using OpenMM platform %s\n",
    //        context.getPlatform().getName().c_str());

    // Set starting positions of the atoms. Leave time and velocity zero.
    context.setPositions(initPosInNm);
    context.setVelocities(initVelocities);

    data_out << "time (ps)    "
             << "KE (kJ/mol)    "
             << "PE (kJ/mol)    "
             << "ET (kJ/mol)" << endl;
    // Simulate.
    remove("NAFlex.pdb");
    remove("NAFlexVel.txt");
    for (int frameNum = 1; frameNum <= 1; frameNum++)
    {
        // Output current state information.
        State state = context.getState(State::Positions | State::Forces | State::Energy | State::Velocities);
        vector<Vec3> forces = state.getForces();
        const double timeInPs = state.getTime();
        double KE = state.getKineticEnergy();
        double PE = state.getPotentialEnergy();
        data_out << setw(13) << left << timeInPs;
        data_out << setw(15) << left << fixed << setprecision(5) << KE;
        data_out << setw(15) << left << fixed << setprecision(5) << PE;
        data_out << setw(15) << left << fixed << setprecision(5) << PE + KE << endl;
        string pdbfile("NAFlex.pdb");
        string velfile("NAFlexVel.txt");
        writePdbFrame(frameNum, state, pdbfile); // output coordinates
        writeVelocites(frameNum, state, velfile);
        // Advance state many steps at a time, for efficient use of OpenMM.
        integrator.step(1); // (use a lot more than this normally)
        if (frameNum == 90000)
        {
            data_out << setw(13) << left << timeInPs;
            data_out << setw(15) << left << fixed << setprecision(5) << KE;
            data_out << setw(15) << left << fixed << setprecision(5) << PE;
            data_out << setw(15) << left << fixed << setprecision(5) << PE + KE << endl;
            writePdbFrame(frameNum, state, pdbfile); // output coordinates
            writeVelocites(frameNum, state, velfile);
        }
    }
    data_out.unsetf(ios::fixed);
}

int main()
{
    try
    {
        registerFlexiBLEReferenceKernelFactories();
        simulateNeon();
        return 0; // success!
    }
    // Catch and report usage and runtime errors detected by OpenMM and fail.
    catch (const std::exception &e)
    {
        printf("EXCEPTION: %s\n", e.what());
        return 1; // failure!
    }
}