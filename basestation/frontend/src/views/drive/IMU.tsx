import { useState, useEffect } from 'react';
import styles from './IMU.module.scss';
import { ArtificialHorizon } from '../../components/artificialHorizon/artificialHorizon';

import { useTopic } from '../../socket/useTopic';
import { type ReceiveIMU } from '../../socket/data/imu';

function IMU() {
  const [roll, setRoll] = useState<number>(0);
  const [pitch, setPitch] = useState<number>(0);
  const [yaw, setYaw] = useState<number>(0);

  const imu = useTopic<ReceiveIMU>('IMU');
  useEffect(() => {
    const start = performance.now();
    const id = setInterval(() => {
      const t = (performance.now() - start) / 1000;
      setRoll(70 * Math.sin(t));
      setPitch(80 * Math.sin(t * 0.7));
      setYaw(180 * Math.sin(t));
    }, 30);
    return () => clearInterval(id);
  }, []);

  return <Horizon roll={roll} pitch={pitch} yaw={yaw} />;
}

type HorizonProps = {
  roll: number;
  pitch: number;
  yaw: number;
};

export function Horizon({ roll, pitch, yaw }: HorizonProps) {
  return (
    <div id='IMU'>
      <div className={styles.horizon}>
        <section className={styles.horizon__title}>
          <h2>Horizon</h2>
        </section>

        <section className={styles.horizon__container}>
          <ArtificialHorizon pitch={pitch} roll={roll} />
          <section className={styles.horizon__metrics}>
            <div className={styles.horizon__metric}>
              <h3>Roll</h3>
              <p>{roll.toFixed(2)}°</p>
            </div>
            <div className={styles.horizon__metric}>
              <h3>Pitch</h3>
              <p>{pitch.toFixed(2)}°</p>
            </div>
            <div className={styles.horizon__metric}>
              <h3>Yaw</h3>
              <p>{yaw.toFixed(2)}°</p>
            </div>
          </section>
        </section>
      </div>
    </div>
  );
}

export default IMU;
