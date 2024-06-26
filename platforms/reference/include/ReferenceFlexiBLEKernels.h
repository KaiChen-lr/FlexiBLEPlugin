#ifndef REFERENCE_FLEXIBLE_KERNELS_H_
#define REFERENCE_FLEXIBLE_KERNELS_H_

/* -------------------------------------------------------------------------- *
 *                      FlexiBLE QM/MM Boundary Potential                     *
 *                          ========================                          *
 *                                                                            *
 * An OpenMM plugin for FlexiBLE force calculation                            *
 *                                                                            *
 * Copyright (c) 2023 Kai Chen, William Glover's group                        *
 * -------------------------------------------------------------------------- */

#include "FlexiBLEKernels.h"
#include "openmm/Platform.h"
#include <vector>
#include <array>
#include <map>
#include <bitset>
#include <unordered_set>
#include <string>

namespace FlexiBLE
{
    struct gInfo
    {
        double val;
        /**
         * Store the value of the exponential part of pair function
         * aka:
         * 0 (R<0)
         * (alpha*R)^3/(1+alpha*R) (R>=0)
         * */
        double der;
        /**
         * Store the derivative for it.
         * d(val)/dR=
         * (3*alpha^3*R^2)/(1+alpha*R)-(alpha^4*R^3)/(1+alpha*R)^2
         * */
    };

    /**
     * This kernel is invoked by FlexiBLEForce to calculate the forces acting on the system.
     */
    class ReferenceCalcFlexiBLEForceKernel : public CalcFlexiBLEForceKernel
    {
    public:
        ReferenceCalcFlexiBLEForceKernel(std::string name, const OpenMM::Platform &platform) : CalcFlexiBLEForceKernel(name, platform) {}
        /**
         * Initialize the kernel.
         * @param context    FlexiBLE needs the information of topology
         * @param system     the System this kernel will be applied to
         * @param force      the FlexiBLEForce this kernel will be used for
         */

        // void GroupingMolecules(Context &context, vector<int> QMIndices, vector<int> MoleculeInit);

        void initialize(const OpenMM::System &system, const FlexiBLEForce &force);
        /**
         * Execute the kernel to calculate the forces and/or energy.
         *
         * @param context        the context in which to execute this kernel
         * @param includeForces  true if forces should be calculated
         * @param includeEnergy  true if the energy should be calculated
         * @return the potential energy due to the force
         */
        double execute(OpenMM::ContextImpl &context, bool includeForces, bool includeEnergy);
        /**
         * Copy changed parameters over to a context.
         *
         * @param context    the context to copy parameters to
         * @param force      the FlexiBLEForce to copy the parameters from
         */
        void copyParametersToContext(OpenMM::ContextImpl &context, const FlexiBLEForce &force);

        std::vector<double> Calc_VecMinus(std::vector<double> lhs, std::vector<double> rhs);
        double Calc_VecDot(std::vector<double> lhs, std::vector<double> rhs);
        double Calc_VecMod(std::vector<double> lhs);
        std::vector<double> Calc_VecSum(std::vector<double> lhs, std::vector<double> rhs);
        std::vector<double> Calc_COM(std::vector<OpenMM::Vec3> Coordinates, int QMFlag, int group, int index);
        // Calculate the distance between atom and the boundary center
        void Calc_r(std::vector<std::pair<int, double>> &rCA, std::vector<std::vector<double>> &rCA_Vec, std::vector<OpenMM::Vec3> Coordinates, int iGroup, int TargetAtom, std::vector<std::vector<double>> &drCA);
        // Calculate the derivative of r over coordinates
        void Calc_dr(int iGroup, int AtomDragged, std::vector<std::pair<int, double>> rCA, std::vector<std::vector<double>> rCA_Vec, std::vector<std::vector<double>> &drCA);
        // This function is here to test the reordering part with function "execute".
        void TestReordering(int Switch, int GroupIndex, int DragIndex, std::vector<OpenMM::Vec3> coor, std::vector<std::pair<int, double>> rAtom, std::vector<double> COM);

        void TestPairFunc(int EnableTestOutput, std::vector<std::vector<gInfo>> gExpPart);

        void TestVal(double Nume, double Deno);

        // Calculates the exponential part, and the derivative over Rij(R)
        double CalcPairExpPart(double alpha, double R, double &der);

        // Calculate the penalty function based on given arrangement, and also the derivative over Ri or Rj
        double CalcPenalFunc(std::vector<int> seq, int QMSize, std::vector<std::vector<FlexiBLE::gInfo>> g, std::vector<double> &DerList, std::vector<std::pair<int, double>> rC_Atom, double h, int part);

        // Find the child node based on the given parent node
        void ProdChild(std::unordered_set<std::string> &Nodes, std::string InputNode, double h, int QMSize, int LB, std::vector<std::vector<FlexiBLE::gInfo>> g, std::vector<double> &DerList, std::vector<std::pair<int, double>> rC_Atom, double &Energy);

        int FindRepeat(std::unordered_set<std::string> Nodes, std::string InputNode);

        void TestNumeDeno(int EnableValOutput, double Nume, std::vector<double> h_list, double alpha, double h, double scale, int QMSize, int MMSize, std::vector<double> NumeForce, std::vector<double> DenoForce, double DenoNow, double DenoLast, std::vector<OpenMM::Vec3> Forces);

    private:
        class InternalInfo;
        std::vector<std::vector<InternalInfo>> QMGroups;
        std::vector<std::vector<InternalInfo>> MMGroups;
        std::vector<int> AssignedAtomIndex;
        std::vector<double> Coefficients;
        std::vector<double> COM;
        int BoundaryShape = 0;
        std::vector<std::vector<double>> BoundaryParameters;
        int EnableTestOutput = 0;
        int EnableValOutput = 0;
        std::vector<double> hThre;
        // std::vector<double> IterGamma;
        std::vector<int> FlexiBLEMaxIt;
        std::vector<double> IterScales;
        int CutoffMethod = 0;
        double T = 300;
        double SystemTotalMass = 0.0;
        // double time_total = 0.0;
        // double find_replica = 0.0;
        // double produce_nodes = 0.0;
        // double time_calc = 0.0;
        // double nodeConvert = 0.0;
    };
    class ReferenceCalcFlexiBLEForceKernel::InternalInfo
    {
    public:
        std::vector<int> Indices;
        std::vector<double> AtomMasses;
    };
} // namespace FlexiBLE

#endif /*REFERENCE_FLEXIBLE_KERNELS_H_*/
