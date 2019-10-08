// Copyright (c) 2012-2019 University of Lyon and CNRS (France).
// All rights reserved.
//
// This file is part of MEPP2; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 3 of
// the License, or (at your option) any later version.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#include "FEVV/Wrappings/Graph_traits_aif.h"
#include "FEVV/Wrappings/Geometry_traits_aif.h"
#include "FEVV/Wrappings/Graph_properties_aif.h"
#include "FEVV/Wrappings/properties_aif.h"

#include "FEVV/DataStructures/AIF/AIFMesh.hpp"
#include "FEVV/DataStructures/AIF/AIFMeshReader.hpp"
#include "FEVV/DataStructures/AIF/AIFMeshWriter.hpp"

#include "FEVV/Operators/AIF/similarity.hpp"
#include "FEVV/Operators/AIF/collapse_edge.hpp"

#include "FEVV/Tools/IO/FileUtilities.hpp"

#include <vector>
#include <set>

using namespace FEVV::DataStructures::AIF;

int
main(int narg, char **argv)
{
  typedef AIFMeshReader reader_type;
  typedef AIFMeshWriter writer_type;
  typedef boost::graph_traits< reader_type::output_type > GraphTraits;
  typedef FEVV::Geometry_traits<AIFMeshT> GeometryTraits;

  typedef typename GraphTraits::vertex_iterator vertex_iterator;
  typedef typename GraphTraits::vertex_descriptor vertex_descriptor;
  typedef typename GraphTraits::edge_iterator edge_iterator;
  typedef typename GraphTraits::edge_descriptor edge_descriptor;
  typedef typename GraphTraits::face_iterator face_iterator;
  typedef typename GraphTraits::face_descriptor face_descriptor;
  /////////////////////////////////////////////////////////////////////////////
  if(narg < 2 || narg > 6)
  {
    std::cerr << "Cannot proceed arguments. Please use " << argv[0]
              << " meshfile colorize_mesh [remove_isolated_elements "
      "[resolve_vertices_with_similar_incident_edges [make_2_mani_not_2_mani]]]"
              << std::endl;
    return -1;
  }
  std::string input_file_path(argv[1]);
  std::string colorize_mesh(argv[2]);
  std::string remove_isolated_elements(((narg > 3) ? argv[3] : "n"));
  std::string resolve_vertices_with_similar_incident_edges(
      ((narg > 4) ? argv[4] : "n"));
  //std::string resolve_similar_faces(((narg > 5) ? argv[5] : "n"));
  std::string make_2_mani_not_2_mani(((narg > 5) ? argv[5] : "n"));
  reader_type my_reader;
  // reader_type::output_type  m;
  reader_type::ptr_output ptr_mesh;
  if (colorize_mesh != "y" && colorize_mesh != "n" &&
    colorize_mesh != "Y" && colorize_mesh != "N")
  {
    std::cerr << "Cannot understand input for colorizing output mesh. "
      "Please use either \"y\" or \"n\" "
      << std::endl;
    return -1;
  }
  if(remove_isolated_elements != "y" && remove_isolated_elements != "n" &&
     remove_isolated_elements != "Y" && remove_isolated_elements != "N")
  {
    std::cerr << "Cannot understand input for removing of not isolated "
                 "element. Please use either \"y\" or \"n\" "
              << std::endl;
    return -1;
  }
  if (resolve_vertices_with_similar_incident_edges != "y" && resolve_vertices_with_similar_incident_edges != "n" &&
    resolve_vertices_with_similar_incident_edges != "Y" && resolve_vertices_with_similar_incident_edges != "N")
  {
    std::cerr << "Cannot understand input for resolving similar/duplicate incident edges. "
      "Please use either \"y\" or \"n\" "
      << std::endl;
    return -1;
  }
  //if (resolve_similar_faces != "y" && resolve_similar_faces != "n" &&
  //  resolve_similar_faces != "Y" && resolve_similar_faces != "N")
  //{
  //  std::cerr << "Cannot understand input for resolving similar/duplicate faces. "
  //    "Please use either \"y\" or \"n\" "
  //    << std::endl;
  //  return -1;
  //}
  if (make_2_mani_not_2_mani != "y" && make_2_mani_not_2_mani != "n" &&
    make_2_mani_not_2_mani != "Y" && make_2_mani_not_2_mani != "N")
  {
    std::cerr << "Cannot understand input for resolving not 2 manifold elements. "
      "Please use either \"y\" or \"n\" "
      << std::endl;
    return -1;
  }
  /////////////////////////////////////////////////////////////////////////////
  try
  {
    ptr_mesh = my_reader.read(input_file_path);
  }
  catch(const std::invalid_argument &e)
  {
    std::cerr << "Invalid Argument Exception catch while reading "
              << input_file_path << " :" << std::endl
              << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  catch(const std::ios_base::failure &e)
  {
    std::cerr << "IOS Failure Exception catch while reading " << input_file_path
              << " :" << std::endl
              << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  catch(const std::length_error &le)
  {
    std::cerr << "[AIF] Exception caught while reading input file "
              << input_file_path << ": " << le.what() << std::endl;
    BOOST_ASSERT_MSG(false, "[AIF] Exception caught while reading input file.");
  }
  /////////////////////////////////////////////////////////////////////////////
  const AIFMeshT::Vector red(255, 0, 0);
  const AIFMeshT::Vector green(0, 255, 0);
  const AIFMeshT::Vector blue(0, 0, 255);
  const AIFMeshT::Vector white(255, 255, 255);
  if (colorize_mesh == "y" || colorize_mesh == "Y")
  {
    if (!ptr_mesh->isPropertyMap< AIFMeshT::vertex_type::ptr >("v:color"))
    { // create a property map to store vertex colors if not already created
      ptr_mesh->AddPropertyMap< AIFMeshT::vertex_type::ptr, AIFMeshT::Vector >("v:color");
      if (!ptr_mesh->isPropertyMap< AIFMeshT::vertex_type::ptr >("v:color"))
        throw std::runtime_error("Failed to create vertex-color property map.");
    }
    if (!ptr_mesh->isPropertyMap< AIFMeshT::edge_type::ptr >("e:color"))
    { // create a property map to store edge colors if not already created
      ptr_mesh->AddPropertyMap< AIFMeshT::edge_type::ptr, AIFMeshT::Vector >("e:color");
      if (!ptr_mesh->isPropertyMap< AIFMeshT::edge_type::ptr >("e:color"))
        throw std::runtime_error("Failed to create edge-color property map.");
    }
    if (!ptr_mesh->isPropertyMap< AIFMeshT::face_type::ptr >("f:color"))
    { // create a property map to store face colors if not already created
      ptr_mesh->AddPropertyMap< AIFMeshT::face_type::ptr, AIFMeshT::Vector >("f:color");
      if (!ptr_mesh->isPropertyMap< AIFMeshT::face_type::ptr >("f:color"))
        throw std::runtime_error("Failed to create face-color property map.");
    }
  }
  if (make_2_mani_not_2_mani == "y" || make_2_mani_not_2_mani == "Y")
  {
    if (!ptr_mesh->isPropertyMap< AIFMeshT::face_type::ptr >("f:2_manifold_component_seg"))
    { // create a property map to store face colors if not already created
      ptr_mesh->AddPropertyMap< AIFMeshT::face_type::ptr, int >("f:2_manifold_component_seg");
      if (!ptr_mesh->isPropertyMap< AIFMeshT::face_type::ptr >("f:2_manifold_component_seg"))
        throw std::runtime_error("Failed to create face 2-manifold components property map.");
    }
    // compute the face segmentation
    std::set<face_descriptor> set_current_component, 
                              all_faces(faces(*ptr_mesh).first, faces(*ptr_mesh).second); // not yet processed
    int current_id = 0;
    set_current_component.insert(*all_faces.begin());
    all_faces.erase(all_faces.begin());
    while (!set_current_component.empty())
    {
      face_descriptor current_f = *set_current_component.begin();
      ptr_mesh->SetProperty< AIFMeshT::face_type::ptr, int >(
        "f:2_manifold_component_seg", current_f->GetIndex(), current_id);

      set_current_component.erase(set_current_component.begin());

      auto vector_faces = AIFHelpers::adjacent_faces(current_f, true);
      auto iter_f = vector_faces.begin(), iter_f_end = vector_faces.end();
      for( ; iter_f!=iter_f_end; ++iter_f)
        if (all_faces.find(*iter_f) != all_faces.end())
        {
          edge_descriptor e_tmp = AIFHelpers::common_edge(current_f, *iter_f);
          if (AIFHelpers::is_2_manifold_edge(e_tmp))
          {
            set_current_component.insert(*iter_f);
            all_faces.erase(*iter_f);
          }
        }

      if (set_current_component.empty() && !all_faces.empty())
      {
        set_current_component.insert(*all_faces.begin());
        all_faces.erase(all_faces.begin());
        ++current_id;
      }
    }
    std::cout << "segmented into " << (current_id + 1) << " 2-manifold components." << std::endl;
  }
  /////////////////////////////////////////////////////////////////////////////
  // 1 - Count number of isolated/complex/normal elements

  // VERTICES
  auto iterator_pair_v = vertices(*ptr_mesh);
  vertex_iterator vi = iterator_pair_v.first;
  vertex_iterator vi_end = iterator_pair_v.second;
  int nb_isolated_vertices = 0, nb_cut_vertices = 0,
      nb_vertices_with_similar_incident_edges = 0;
  std::vector< vertex_descriptor > v_to_remeove;
  std::vector< vertex_descriptor > cut_vertices;
  for(; vi != vi_end; ++vi)
  {
    if(AIFHelpers::is_isolated_vertex(*vi))
    {
      ++nb_isolated_vertices;

      if(remove_isolated_elements == "y" || remove_isolated_elements == "Y")
        v_to_remeove.push_back(*vi);
      else
      {
        if (colorize_mesh == "y" || colorize_mesh == "Y")
          ptr_mesh->SetProperty< AIFMeshT::vertex_type::ptr, AIFMeshT::Vector >(
              "v:color", (*vi)->GetIndex(), red);
      }
    }
    else if(AIFHelpers::is_cut_vertex(*vi))
    {
      ++nb_cut_vertices;
      if (make_2_mani_not_2_mani == "y" || make_2_mani_not_2_mani == "Y")
        cut_vertices.push_back(*vi);
      else
        if (colorize_mesh == "y" || colorize_mesh == "Y")
          ptr_mesh->SetProperty< AIFMeshT::vertex_type::ptr, AIFMeshT::Vector >(
            "v:color", (*vi)->GetIndex(), blue);
    }
    else if(FEVV::Operators::contains_similar_incident_edges(
                *vi, *ptr_mesh))
    {

      ++nb_vertices_with_similar_incident_edges;

      if(resolve_vertices_with_similar_incident_edges == "y" ||
         resolve_vertices_with_similar_incident_edges == "Y")
        FEVV::Operators::resolve_similar_incident_edges(*vi, *ptr_mesh);
      else
        if (colorize_mesh == "y" || colorize_mesh == "Y")
          ptr_mesh->SetProperty< AIFMeshT::vertex_type::ptr, AIFMeshT::Vector >(
              "v:color", (*vi)->GetIndex(), blue);
    }
    else
    {
      if (colorize_mesh == "y" || colorize_mesh == "Y")
        ptr_mesh->SetProperty< AIFMeshT::vertex_type::ptr, AIFMeshT::Vector >(
            "v:color", (*vi)->GetIndex(), green);
    }
  }

  // EDGES
  auto iterator_pair_e = edges(*ptr_mesh);
  edge_iterator ei = iterator_pair_e.first;
  edge_iterator ei_end = iterator_pair_e.second;
  int nb_isolated_edges = 0, nb_dangling_edges = 0, nb_complex_edges = 0, 
      nb_border_edges = 0;
  std::vector< edge_descriptor > e_to_remeove;
  std::vector< edge_descriptor > dangling_edges, complex_edges;
  for(; ei != ei_end; ++ei)
  {
    if(AIFHelpers::is_isolated_edge(*ei))
    {
      ++nb_isolated_edges;
      if(remove_isolated_elements == "y" || remove_isolated_elements == "Y")
        e_to_remeove.push_back(*ei);
      else
      {
        if (colorize_mesh == "y" || colorize_mesh == "Y")
          ptr_mesh->SetProperty< AIFMeshT::edge_type::ptr, AIFMeshT::Vector >(
              "e:color", (*ei)->GetIndex(), red);
      }
    }
    else if(AIFHelpers::is_dangling_edge(*ei))
    {
      ++nb_dangling_edges;
      if (make_2_mani_not_2_mani == "y" || make_2_mani_not_2_mani == "Y")
        dangling_edges.push_back(*ei);
      else
      {
        if (colorize_mesh == "y" || colorize_mesh == "Y")
          ptr_mesh->SetProperty< AIFMeshT::edge_type::ptr, AIFMeshT::Vector >(
            "e:color", (*ei)->GetIndex(), red);
      }
    }
    else if(AIFHelpers::is_complex_edge(*ei))
    {
      ++nb_complex_edges;
      if (make_2_mani_not_2_mani == "y" || make_2_mani_not_2_mani == "Y")
        complex_edges.push_back(*ei);
      else
      {
        if (colorize_mesh == "y" || colorize_mesh == "Y")
          ptr_mesh->SetProperty< AIFMeshT::edge_type::ptr, AIFMeshT::Vector >(
            "e:color", (*ei)->GetIndex(), blue);
      }
    }
    else if (AIFHelpers::is_surface_border_edge(*ei))
    {
      ++nb_border_edges;
      if (colorize_mesh == "y" || colorize_mesh == "Y")
        ptr_mesh->SetProperty< AIFMeshT::edge_type::ptr, AIFMeshT::Vector >(
            "e:color", (*ei)->GetIndex(), white);
    }
    else
    {
      if (colorize_mesh == "y" || colorize_mesh == "Y")
        ptr_mesh->SetProperty< AIFMeshT::edge_type::ptr, AIFMeshT::Vector >(
            "e:color", (*ei)->GetIndex(), green);
    }
  }

  // FACES
  auto iterator_pair_f = faces(*ptr_mesh);
  face_iterator fi = iterator_pair_f.first;
  face_iterator fi_end = iterator_pair_f.second;
  int nb_isolated_faces = 0, nb_degenerated_faces = 0;
  std::vector< face_descriptor > f_to_remeove;
  for(; fi != fi_end; ++fi)
  {
    if(AIFHelpers::is_isolated_face(*fi))
    {
      ++nb_isolated_faces;
      if(remove_isolated_elements == "y" || remove_isolated_elements == "Y")
        f_to_remeove.push_back(*fi);
      else
      {
        if (colorize_mesh == "y" || colorize_mesh == "Y")
          ptr_mesh->SetProperty< AIFMeshT::face_type::ptr, AIFMeshT::Vector >(
              "f:color", (*fi)->GetIndex(), red);
      }
    }
    else
    {
      if (colorize_mesh == "y" || colorize_mesh == "Y")
        ptr_mesh->SetProperty< AIFMeshT::face_type::ptr, AIFMeshT::Vector >(
            "f:color", (*fi)->GetIndex(), green);
    }

    if (AIFHelpers::is_degenerated_face(*fi))
      ++nb_degenerated_faces;
  }
  /////////////////////////////////////////////////////////////////////////////
  // 2 - Remove isolated elements
  std::string suffix = "";
  if(remove_isolated_elements == "y" || remove_isolated_elements == "Y")
  {
    if(nb_isolated_vertices != 0 || nb_isolated_edges != 0 ||
       nb_isolated_faces != 0)
    {
      AIFHelpers::remove_faces(
          f_to_remeove.begin(), f_to_remeove.end(), ptr_mesh);
      AIFHelpers::remove_edges(
          e_to_remeove.begin(), e_to_remeove.end(), ptr_mesh);
      AIFHelpers::remove_vertices(
          v_to_remeove.begin(), v_to_remeove.end(), ptr_mesh);
      suffix = "_without_isolated_elmt";
      if(resolve_vertices_with_similar_incident_edges == "y" ||
         resolve_vertices_with_similar_incident_edges == "Y")
        suffix += "_and_similar_incident_edges";
    }
    else if(resolve_vertices_with_similar_incident_edges == "y" ||
            resolve_vertices_with_similar_incident_edges == "Y")
      suffix = "_without_similar_incident_edges";

    if (make_2_mani_not_2_mani == "y" || make_2_mani_not_2_mani == "Y")
      suffix += "_without_non_manifold_elm";
  }
  else if (resolve_vertices_with_similar_incident_edges == "y" ||
    resolve_vertices_with_similar_incident_edges == "Y")
  {
    suffix = "_without_similar_incident_edges";
    if (make_2_mani_not_2_mani == "y" || make_2_mani_not_2_mani == "Y")
      suffix += "_without_non_manifold_elm";
  }
  else if( make_2_mani_not_2_mani == "y" || make_2_mani_not_2_mani == "Y")
    suffix = "_without_non_manifold_elm";
  /////////////////////////////////////////////////////////////////////////////
  // 3 - Process non-2-manifold elements
  auto pos_pm = get(boost::vertex_point, *ptr_mesh);

  // remove dangling edges
  auto iter_e = dangling_edges.begin(), iter_e_end = dangling_edges.end();
  while (iter_e != iter_e_end)
  {
    if(degree(source(*iter_e, *ptr_mesh), *ptr_mesh)==1)
      FEVV::Operators::collapse_edge_keep_target(*ptr_mesh, *iter_e);
    else if (degree(target(*iter_e, *ptr_mesh), *ptr_mesh) == 1)
      FEVV::Operators::collapse_edge_keep_source(*ptr_mesh, *iter_e);
    else
      FEVV::Operators::collapse_edge_keep_source(*ptr_mesh, *iter_e);
    
    ++iter_e;
  }

  // decomplexify complex edges
  iter_e = complex_edges.begin(), iter_e_end = complex_edges.end();
  while (iter_e != iter_e_end)
  {
    // complex edges
    std::cout << "degree of current complex edge: " << degree(*iter_e, *ptr_mesh) << std::endl; // debug
    auto faces_range_pair = in_edges(*iter_e, *ptr_mesh); // get incident faces
    std::set<face_descriptor> current_faces(faces_range_pair.first, faces_range_pair.second),
                              next_faces;
    std::map<int, edge_descriptor> face_id_to_edge;
    std::map<vertex_descriptor, vertex_descriptor> v_old_to_v_new;
    bool first = true;
    while (!current_faces.empty())
    {
      face_descriptor current_f = *current_faces.begin();
      int current_id = ptr_mesh->GetProperty< AIFMeshT::face_type::ptr, int >(
        "f:2_manifold_component_seg", current_f->GetIndex());
      
      if (first)
      { // the first component keep the initial complex edge
        face_id_to_edge.insert(std::make_pair(current_id, *iter_e));
      }
      else
      { // other components use a duplicate
        std::cout << "create a new edge" << std::endl; // debug
        edge_descriptor e_tmp = AIFHelpers::add_edge(*ptr_mesh);
        vertex_descriptor s_tmp = AIFHelpers::add_vertex(*ptr_mesh),
                          t_tmp = AIFHelpers::add_vertex(*ptr_mesh);
        AIFHelpers::link_vertex_and_edge(s_tmp, e_tmp, AIFHelpers::vertex_pos::FIRST);
        AIFHelpers::link_vertex_and_edge(t_tmp, e_tmp, AIFHelpers::vertex_pos::SECOND);

        face_id_to_edge.insert(std::make_pair(current_id, e_tmp));
      }

      auto iter_f = current_faces.begin(), iter_f_end = current_faces.end();
      for (; iter_f != iter_f_end; ++iter_f)
      {
        if (ptr_mesh->GetProperty< AIFMeshT::face_type::ptr, int >(
          "f:2_manifold_component_seg", (*iter_f)->GetIndex()) != current_id)
          next_faces.insert(*iter_f);
        else
        {
          if (first)
            continue;

          // for other components: update incidence relationships
          edge_descriptor pe = AIFHelpers::get_edge_of_face_before_edge(*iter_f, *iter_e);
          edge_descriptor ae = AIFHelpers::get_edge_of_face_after_edge(*iter_f, *iter_e);

          AIFHelpers::remove_edge(*iter_f, *iter_e);
          AIFHelpers::remove_face(*iter_e, *iter_f);
          AIFHelpers::add_edge_to_face_after_edge(*iter_f, pe, face_id_to_edge[current_id]);
          AIFHelpers::add_face(face_id_to_edge[current_id], *iter_f);

          vertex_descriptor v_pe_old = AIFHelpers::common_vertex(pe, *iter_e);
          vertex_descriptor v_ae_old = AIFHelpers::common_vertex(ae, *iter_e);

          AIFHelpers::remove_edge(v_pe_old, pe);
          AIFHelpers::remove_edge(v_ae_old, ae);

          if (degree(face_id_to_edge[current_id], *ptr_mesh) == 1)
          { // first processed face
            put(pos_pm, source(face_id_to_edge[current_id], *ptr_mesh), get(pos_pm, v_pe_old));
            put(pos_pm, target(face_id_to_edge[current_id], *ptr_mesh), get(pos_pm, v_ae_old));

            AIFHelpers::add_edge(source(face_id_to_edge[current_id], *ptr_mesh), pe);
            AIFHelpers::add_vertex(pe, source(face_id_to_edge[current_id], *ptr_mesh), AIFHelpers::vertex_position(pe, v_pe_old));

            AIFHelpers::add_edge(target(face_id_to_edge[current_id], *ptr_mesh), ae);
            AIFHelpers::add_vertex(ae, target(face_id_to_edge[current_id], *ptr_mesh), AIFHelpers::vertex_position(ae, v_ae_old));

            v_old_to_v_new[v_pe_old] = source(face_id_to_edge[current_id], *ptr_mesh);
            v_old_to_v_new[v_ae_old] = target(face_id_to_edge[current_id], *ptr_mesh);
          }
          else
          {
            AIFHelpers::add_edge(v_old_to_v_new[v_pe_old], pe);
            AIFHelpers::add_vertex(pe, v_old_to_v_new[v_pe_old], AIFHelpers::vertex_position(pe, v_pe_old));

            AIFHelpers::add_edge(v_old_to_v_new[v_ae_old], ae);
            AIFHelpers::add_vertex(ae, v_old_to_v_new[v_ae_old], AIFHelpers::vertex_position(ae, v_ae_old));
          }
        }
      }
      first = false;
      current_faces.swap(next_faces);
      next_faces.clear();
    }

    ++iter_e;
  }
#if 0
  // duplicate cut vertices
  auto iter_v = cut_vertices.begin(), iter_v_end = cut_vertices.end();
  while (iter_v != iter_v_end)
  {
    auto face_range = AIFHelpers::incident_faces(*iter_v);
    auto it_f = face_range.begin();
    for (; it_f != face_range.end(); ++it_f)
    {
      // faces segmented with the same id need to replace *iter_v by a new vertex
      // except for the first piece of surface
      
      vertex_descriptor v_tmp = AIFHelpers::add_vertex(*ptr_mesh);

    }

    ++iter_v;
  }
#endif
  /////////////////////////////////////////////////////////////////////////////
  // 4 - Save corrected mesh
  writer_type my_writer;
  try
  {
    if(FEVV::FileUtils::get_file_extension(input_file_path) == ".ply")
      my_writer.write(ptr_mesh,
                      FEVV::FileUtils::get_file_name(input_file_path) + suffix +
                          ".off");
    else
      my_writer.write(ptr_mesh,
                      FEVV::FileUtils::get_file_name(input_file_path) + suffix +
                          FEVV::FileUtils::get_file_extension(input_file_path));
  }
  catch(...)
  {
    std::cout << "writing failed";
    return -1;
  }
  /////////////////////////////////////////////////////////////////////////////
  std::cout << "The mesh file named "
            << FEVV::FileUtils::get_file_name(input_file_path) +
                   FEVV::FileUtils::get_file_extension(input_file_path);
  if(nb_isolated_vertices > 0 || nb_cut_vertices > 0 || nb_isolated_edges > 0 ||
     nb_dangling_edges > 0 || nb_complex_edges > 0)
    std::cout << " is not 2-manifold " << std::endl;
  else
    std::cout << " is 2-manifold" << std::endl;

  std::string prefix =
      (remove_isolated_elements == "y" || remove_isolated_elements == "Y")
          ? "Number of removed isolated"
          : "Number of isolated";
  std::cout << prefix << " vertices: " << nb_isolated_vertices << std::endl;
  std::cout << prefix << " edges: " << nb_isolated_edges << std::endl;
  std::cout << prefix << " faces: " << nb_isolated_faces << std::endl;
  /////////////////////////////////////////////////////////////////////////////
  prefix = (resolve_vertices_with_similar_incident_edges == "y" ||
            resolve_vertices_with_similar_incident_edges == "Y")
               ? "Number of resolved vertices with similar incident edges: "
               : "Number of vertices with similar incident edges: ";
  std::cout << prefix << nb_vertices_with_similar_incident_edges << std::endl;
  /////////////////////////////////////////////////////////////////////////////
  std::cout << "Number of cut vertices: " << nb_cut_vertices << std::endl;
  std::cout << "Number of dangling edges: " << nb_dangling_edges << std::endl;
  std::cout << "Number of complex edges: " << nb_complex_edges << std::endl;
  std::cout << "Number of surface border edges: " << nb_border_edges << std::endl;
  std::cout << "Number of degenerated faces: " << nb_degenerated_faces << std::endl;
  /////////////////////////////////////////////////////////////////////////////
  system("pause");
  return 0;
}
