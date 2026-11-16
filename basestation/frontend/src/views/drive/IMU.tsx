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
    <div id='IMU' className={styles.module}>
      <div className={styles.horizon}>
        <h2 className={styles.horizon__title}>Horizon</h2>
        <section className={styles.horizon__container}>
          <div className={styles.horizon__instrument}>
            <ArtificialHorizon pitch={pitch} roll={roll} size={320} />
          </div>
          <section className={styles.horizon__metrics}>
            <div className={styles.horizon__metric}>
              <h3>Pitch</h3>
              <p>{pitch > 0 ? '+' : ''}{pitch.toFixed(1)}°</p>
            </div>
            <div className={styles.horizon__metric}>
              <h3>Roll</h3>
              <p>{roll > 0 ? '+' : ''}{roll.toFixed(1)}°</p>
            </div>
            <div className={`${styles.horizon__metric} ${styles.horizon__yaw}`}>
              <h3>Yaw</h3>
              <p>{yaw.toFixed(1)}°</p>
            </div>
          </section>
        </section>
      </div>
    </div>
  );
}

export default IMU;
