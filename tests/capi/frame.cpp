// Chemfiles, a modern library for chemistry file reading and writing
// Copyright (C) Guillaume Fraux and contributors -- BSD license

#include "catch.hpp"
#include "helpers.hpp"
#include "chemfiles.h"

TEST_CASE("Frame", "[CAPI]") {
    SECTION("Size") {
        CHFL_FRAME* frame = chfl_frame();
        REQUIRE(frame != NULL);

        uint64_t natoms = 10;
        CHECK_STATUS(chfl_frame_atoms_count(frame, &natoms));
        CHECK(natoms == 0);

        CHECK_STATUS(chfl_frame_resize(frame, 4));
        CHECK_STATUS(chfl_frame_atoms_count(frame, &natoms));
        CHECK(natoms == 4);

        CHECK_STATUS(chfl_frame_remove(frame, 0));
        CHECK_STATUS(chfl_frame_atoms_count(frame, &natoms));
        CHECK(natoms == 3);

        CHECK_STATUS(chfl_frame_free(frame));
    }

    SECTION("Step") {
        CHFL_FRAME* frame = chfl_frame();
        REQUIRE(frame != NULL);

        uint64_t step = 0;
        CHECK_STATUS(chfl_frame_step(frame, &step));
        CHECK(step == 0);

        CHECK_STATUS(chfl_frame_set_step(frame, 42));
        CHECK_STATUS(chfl_frame_step(frame, &step));
        CHECK(step == 42);

        CHECK_STATUS(chfl_frame_free(frame));
    }

    SECTION("Positions") {
        CHFL_FRAME* frame = chfl_frame();
        REQUIRE(frame != NULL);
        CHECK_STATUS(chfl_frame_resize(frame, 4));

        uint64_t natoms = 0;
        chfl_vector_t* positions = NULL;
        CHECK_STATUS(chfl_frame_positions(frame, &positions, &natoms));
        CHECK(natoms == 4);

        for (size_t i=0; i<natoms; i++) {
            for (size_t j=0; j<3; j++) {
                positions[i][j] = static_cast<double>(i * j);
            }
        }

        positions = NULL;
        CHECK_STATUS(chfl_frame_positions(frame, &positions, &natoms));
        for (size_t i=0; i<natoms; i++) {
            for (size_t j=0; j<3; j++) {
                CHECK(positions[i][j] == i * j);
            }
        }

        CHECK_STATUS(chfl_frame_free(frame));
    }

    SECTION("Velocities") {
        CHFL_FRAME* frame = chfl_frame();
        REQUIRE(frame != NULL);
        CHECK_STATUS(chfl_frame_resize(frame, 4));

        bool has_velocities = true;
        CHECK_STATUS(chfl_frame_has_velocities(frame, &has_velocities));
        CHECK(has_velocities == false);
        CHECK_STATUS(chfl_frame_add_velocities(frame));
        CHECK_STATUS(chfl_frame_has_velocities(frame, &has_velocities));
        CHECK(has_velocities == true);

        uint64_t natoms = 0;
        chfl_vector_t* velocities = NULL;
        CHECK_STATUS(chfl_frame_velocities(frame, &velocities, &natoms));
        CHECK(natoms == 4);

        for (size_t i=0; i<natoms; i++) {
            for (size_t j=0; j<3; j++) {
                velocities[i][j] = static_cast<double>(i * j + 1);
            }
        }

        velocities = NULL;
        CHECK_STATUS(chfl_frame_velocities(frame, &velocities, &natoms));
        for (size_t i=0; i<natoms; i++) {
            for (size_t j=0; j<3; j++) {
                CHECK(velocities[i][j] == i * j + 1);
            }
        }

        CHECK_STATUS(chfl_frame_free(frame));
    }

    SECTION("Unit cell") {
        CHFL_FRAME* frame = chfl_frame();
        REQUIRE(frame != NULL);

        // Initial cell
        CHFL_CELL* cell = chfl_cell_from_frame(frame);
        REQUIRE(cell != NULL);

        chfl_vector_t lengths = {0};
        CHECK_STATUS(chfl_cell_lengths(cell, lengths));
        CHECK(lengths[0] == 0.0);
        CHECK(lengths[1] == 0.0);
        CHECK(lengths[2] == 0.0);

        chfl_cell_shape_t shape;
        CHECK_STATUS(chfl_cell_shape(cell, &shape));
        CHECK(shape == CHFL_CELL_INFINITE);

        CHECK_STATUS(chfl_cell_free(cell));

        // Setting an unit cell
        lengths[0] = 3; lengths[1] = 4; lengths[2] = 5;
        cell = chfl_cell(lengths);
        REQUIRE(cell != NULL);
        CHECK_STATUS(chfl_frame_set_cell(frame, cell));
        CHECK_STATUS(chfl_cell_free(cell));

        cell = chfl_cell_from_frame(frame);
        REQUIRE(cell != NULL);

        CHECK_STATUS(chfl_cell_lengths(cell, lengths));
        CHECK(lengths[0] == 3.0);
        CHECK(lengths[1] == 4.0);
        CHECK(lengths[2] == 5.0);

        CHECK_STATUS(chfl_cell_shape(cell, &shape));
        CHECK(shape == CHFL_CELL_ORTHORHOMBIC);

        CHECK_STATUS(chfl_cell_free(cell));
        CHECK_STATUS(chfl_frame_free(frame));
    }

    SECTION("Add atoms") {
        CHFL_FRAME* frame = chfl_frame();
        REQUIRE(frame != NULL);

        uint64_t natoms = 1000;
        CHECK_STATUS(chfl_frame_atoms_count(frame, &natoms));
        CHECK(natoms == 0);

        CHECK_STATUS(chfl_frame_add_velocities(frame));

        CHFL_ATOM* atom = chfl_atom("U");
        chfl_vector_t position = {2, 3, 4};
        CHECK_STATUS(chfl_frame_add_atom(frame, atom, position, NULL));
        chfl_vector_t velocity = {1, 2, 1};
        CHECK_STATUS(chfl_frame_add_atom(frame, atom, position, velocity));
        CHECK_STATUS(chfl_atom_free(atom));

        CHECK_STATUS(chfl_frame_atoms_count(frame, &natoms));
        CHECK(natoms == 2);

        chfl_vector_t* positions = NULL;
        CHECK_STATUS(chfl_frame_positions(frame, &positions, &natoms));
        CHECK(natoms == 2);
        CHECK(positions[0][0] == 2);
        CHECK(positions[0][1] == 3);
        CHECK(positions[0][2] == 4);

        CHECK(positions[1][0] == 2);
        CHECK(positions[1][1] == 3);
        CHECK(positions[1][2] == 4);

        chfl_vector_t* velocities = NULL;
        CHECK_STATUS(chfl_frame_velocities(frame, &velocities, &natoms));
        CHECK(natoms == 2);

        CHECK(velocities[0][0] == 0);
        CHECK(velocities[0][1] == 0);
        CHECK(velocities[0][2] == 0);

        CHECK(velocities[1][0] == 1);
        CHECK(velocities[1][1] == 2);
        CHECK(velocities[1][2] == 1);

        CHECK_STATUS(chfl_frame_free(frame));
    }

    SECTION("Topology") {
        CHFL_FRAME* frame = chfl_frame();
        REQUIRE(frame != NULL);
        CHECK_STATUS(chfl_frame_resize(frame, 4));

        CHFL_TOPOLOGY* topology = chfl_topology();
        REQUIRE(topology != NULL);

        CHFL_ATOM* Zn = chfl_atom("Zn");
        REQUIRE(Zn != NULL);
        CHFL_ATOM* Ar = chfl_atom("Ar");
        REQUIRE(Ar != NULL);

        CHECK_STATUS(chfl_topology_add_atom(topology, Zn));
        CHECK_STATUS(chfl_topology_add_atom(topology, Ar));
        CHECK_STATUS(chfl_topology_add_atom(topology, Zn));
        CHECK_STATUS(chfl_topology_add_atom(topology, Ar));
        CHECK_STATUS(chfl_atom_free(Zn));
        CHECK_STATUS(chfl_atom_free(Ar));

        CHECK_STATUS(chfl_frame_set_topology(frame, topology));
        CHECK_STATUS(chfl_topology_free(topology));

        topology = chfl_topology_from_frame(frame);
        REQUIRE(topology != NULL);

        uint64_t natoms = 0;
        CHECK_STATUS(chfl_topology_atoms_count(topology, &natoms));
        CHECK(natoms == 4);

        CHFL_ATOM* atom = chfl_atom_from_topology(topology, 1);
        REQUIRE(atom != NULL);

        char name[32] = {0};
        CHECK_STATUS(chfl_atom_name(atom, name, sizeof(name)));
        CHECK(name == std::string("Ar"));

        CHECK_STATUS(chfl_atom_free(atom));
        CHECK_STATUS(chfl_topology_free(topology));
        CHECK_STATUS(chfl_frame_free(frame));
    }

    SECTION("Get atoms") {
        CHFL_FRAME* frame = chfl_frame();
        REQUIRE(frame != NULL);
        CHECK_STATUS(chfl_frame_resize(frame, 10));

        CHFL_ATOM* atom = chfl_atom_from_frame(frame, 1);
        REQUIRE(atom != NULL);

        char name[32] = {0};
        CHECK_STATUS(chfl_atom_name(atom, name, sizeof(name)));
        CHECK(name == std::string(""));
        CHECK_STATUS(chfl_atom_free(atom));

        // Out of bounds access
        atom = chfl_atom_from_frame(frame, 10000);
        CHECK(atom == NULL);

        CHECK_STATUS(chfl_frame_free(frame));
    }

    SECTION("Guess topology") {
        CHFL_TRAJECTORY* trajectory = chfl_trajectory_open("data/xyz/water.xyz", 'r');
        REQUIRE(trajectory != NULL);

        chfl_vector_t lengths = {30, 30, 30};
        CHFL_CELL* cell = chfl_cell(lengths);
        CHECK_STATUS(chfl_trajectory_set_cell(trajectory, cell));
        CHECK_STATUS(chfl_cell_free(cell));

        CHFL_FRAME* frame = chfl_frame();
        REQUIRE(frame != NULL);
        CHECK_STATUS(chfl_trajectory_read(trajectory, frame));

        CHECK_STATUS(chfl_frame_guess_topology(frame));
        CHFL_TOPOLOGY* topology = chfl_topology_from_frame(frame);
        REQUIRE(topology != NULL);

        uint64_t n = 0;
        CHECK_STATUS(chfl_topology_bonds_count(topology, &n));
        CHECK(n == 186);

        CHECK_STATUS(chfl_topology_angles_count(topology, &n));
        CHECK(n == 87);

        CHECK_STATUS(chfl_topology_dihedrals_count(topology, &n));
        CHECK(n == 0);

        CHECK_STATUS(chfl_topology_free(topology));
        CHECK_STATUS(chfl_frame_free(frame));
        CHECK_STATUS(chfl_trajectory_close(trajectory));
    }
}
