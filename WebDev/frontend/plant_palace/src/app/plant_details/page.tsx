"use client"
import { useSearchParams} from 'next/navigation'
import Link from 'next/link';
import styles from "./page.module.css";

interface Harvest{
  date: string;
  amount: string;
}

async function getHarvestData(uri: string): Promise<Harvest[]> {
    const res = await fetch(uri);
    if (!res.ok) {
        throw new Error('Failed to fetch data');
    }
    return res.json();
}

function PlantDetailsComponent({ params }: {params: {name: string, species: string, cultivar: string, stage: string, jpg: string }}) {
  const search_params = useSearchParams();
  const name = search_params.get("name");
  const species = search_params.get("species");
  const cultivar = search_params.get("cultivar");
  const stage = search_params.get("stage");
  const jpg = search_params.get("jpg");

  if (!jpg){
    throw Error
  }

  return (
    <>
      <h1>{name}</h1>
      <div>
        <div>
          <img src={jpg} className="details-image"/>
          <br/>
          Species: {species} <br/>
          Stage: {stage}
        </div>
        <table>
          <tbody>
            <tr>
              <th>Date</th>
              <th>Amount</th>
            </tr>
          </tbody>
        </table>
      </div>
    </>
  );
}

export default PlantDetailsComponent;
