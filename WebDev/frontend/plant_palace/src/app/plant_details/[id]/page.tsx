"use client";
import { useRouter } from "next/navigation";
import Link from "next/link";
import styles from "./page.module.css";
import { useState, useEffect } from "react";

interface Harvest {
  Date: string;
  Amount: string;
}

interface PageParams {
  params: {
    id: string;
  };
}

type Plant = {
  id: number;
  name: string;
  species: string;
  stage: string;
  location: string;
  cultivar?: string;
};

const jpgs_uri = "https://cpsc4910sq24.s3.amazonaws.com/images/";

function determine_photo_uri(species: string, cultivar?: string): string {
  if (species === "Arugula") {
    return jpgs_uri + "arugula.jpg";
  } else if (species === "Bell pepper") {
    return jpgs_uri + "bell-pepper.jpg";
  } else if (species === "Lettuce") {
    if (!cultivar) {
      return "";
    } else if (cultivar === "Butter") {
      return jpgs_uri + "butter-lettuce.jpg";
    } else if (cultivar === "Green leaf") {
      return jpgs_uri + "green-leaf-lettuce.jpg";
    }
    return "";
  } else if (species === "Strawberry") {
    return jpgs_uri + "strawberry.jpg";
  }
  return "";
}

function PlantDetailsComponent({ params }: PageParams) {
  const id = params.id;

  //const router = useRouter();
  const [plantData, setPlantData] = useState<Plant | null>(null);
  const [plantHarvests, setPlantHarvests] = useState<Harvest[] | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    const getPlantData = async () => {
      try {
        const plant_res = await fetch(
          `https://cpsc4910sq24.s3.amazonaws.com/data/plants/${id}.json`,
        );
        if (!plant_res.ok) {
          throw new Error("Failed to fetch data");
        }
        const plant_result = await plant_res.json();
        setPlantData(plant_result);

        const harvest_res = await fetch(
          `https://cpsc4910sq24.s3.amazonaws.com/data/plants/${id}/harvests.json`,
        );
        if (!harvest_res.ok) {
          throw new Error("Failed to fetch data");
        }
        const harvest_result = await harvest_res.json();
        setPlantHarvests(harvest_result);

        setLoading(false);
      } catch (error: any) {
        setError(error.message);
        setLoading(false);
      }
    };

    getPlantData();
  });

  if (loading) {
    return <div>Loading...</div>;
  }

  if (!plantData || !plantHarvests) {
    return <>Error loading data</>;
  }

  if (error) {
    return <div>Error loading data: {error}</div>;
  }

  const species = plantData.species;
  const cultivar = plantData.cultivar;
  const name = plantData.name;
  const stage = plantData.stage;

  console.log("harvests are");
  console.log(plantHarvests);

  return (
    <>
      <div className="main-div">
        <h1>{name}</h1>
        <div className="container">
          <div className="image-container">
            <img
              src={determine_photo_uri(species, cultivar)}
              className="details-image"
              alt="a photo of the plant"
            />
            <br />
            Species: {species} <br />
            Stage: {stage}
          </div>
          <div className="border"></div>
          <div className="table-container">
            <span className="table-title">Past Harvests</span>
            <table className="harvest-table">
              <tbody>
                <tr>
                  <th>Date</th>
                  <th>Amount</th>
                </tr>
                {plantHarvests.map((item: Harvest, index: number) => (
                  <tr key={index}>
                    <td>{item.Date}</td>
                    <td>{item.Amount}</td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        </div>
      </div>
    </>
  );
}

export default PlantDetailsComponent;
