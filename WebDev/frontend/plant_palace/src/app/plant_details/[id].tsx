"use client";
import { useSearchParams } from "next/navigation";
import Link from "next/link";
import styles from "./page.module.css";

interface Harvest {
  date: string;
  amount: string;
}

async function getHarvestData(id: string): Promise<Harvest[]> {
  const uri = `https://cpsc4910sq24.s3.amazonaws.com/data/plants/${id}/harvests.json`;
  const res = await fetch(uri);
  if (!res.ok) {
    throw new Error("Failed to fetch data");
  }
  return res.json();
}

interface PageParams {
  params: {
    id: string;
  };
}

function PlantDetailsComponent({ params }: PageParams) {
  const search_params = useSearchParams();
  const name = search_params.get("name");
  const species = search_params.get("species");
  const cultivar = search_params.get("cultivar");
  const stage = search_params.get("stage");
  const jpg = search_params.get("jpg");
  const id = search_params.get("id");
  return <> hello world </>;
  //const harvest_data = await getHarvestData(id);

  /*
    return (
      <>
        <h1>{name}</h1>
        <div>
          <div>
            <img src={jpg} className="details-image" />
            <br />
            Species: {species} <br />
            Stage: {stage}
          </div>
          <table>
            <tbody>
              <tr>
                <th>Date</th>
                <th>Amount</th>
              </tr>
              {harvest_data.map((item: Harvest, index: number) => (
                <tr key={index}>
                  <td>{item.date}</td>
                  <td>{item.amount}</td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </>
    );
    */
}

export default PlantDetailsComponent;
