import { useId } from "react";

type ArtificialHorizonProps = {
  pitch: number; // Degrees: positive = nose up
  roll: number; // Degrees: negative = visual tilt left, positive = visual tilt right
  size?: number;
};

export function ArtificialHorizon({
  pitch,
  roll,
  size = 200,
}: ArtificialHorizonProps) {
  const clipId = useId();

  // SVG units per degree. Increase for more visible pitch movement.
  const pitchScale = 2;
  const pitchOffset = pitch * pitchScale;

  const pitchMarks = [-30, -20, -10, 10, 20, 30];

  return (
    <svg
      viewBox="-100 -100 200 200"
      width={size}
      height={size}
      role="img"
      aria-label={`Attitude: pitch ${pitch} degrees, roll ${roll} degrees`}
    >
      <defs>
        <clipPath id={clipId}>
          <circle r="90" />
        </clipPath>
      </defs>

      {/* Outer instrument housing */}
      <circle
        r="98"
        fill="#080d11"
        stroke="#30404a"
      />

      {/* Circular window stays stationary */}
      <g clipPath={`url(#${clipId})`}>
        {/* Negative SVG rotation tilts left (counterclockwise). */}
        <g transform={`rotate(${roll})`}>
          {/* Pitch moves along the rotated vertical axis */}
          <g transform={`translate(0 ${pitchOffset})`}>
            {/* Oversized backgrounds prevent gaps during movement */}
            <rect
              x="-500"
              y="-500"
              width="1000"
              height="500"
              fill="#254654"
            />
            <rect
              x="-500"
              y="0"
              width="1000"
              height="500"
              fill="#413d33"
            />

            {/* Horizon line */}
            <line
              x1="-500"
              y1="0"
              x2="500"
              y2="0"
              stroke="#e1e8e9"
              strokeWidth="1"
            />

            {/* Pitch ladder */}
            {pitchMarks.map((angle) => {
              const y = -angle * pitchScale;
              const halfWidth = Math.abs(angle) === 10 ? 30 : 23;

              return (
                <g key={angle}>
                  <line
                    x1={-halfWidth}
                    y1={y}
                    x2={halfWidth}
                    y2={y}
                    stroke="#e1e8e9"
                    strokeWidth="1"
                  />

                  <text
                    x={-halfWidth - 10}
                    y={y}
                    textAnchor="end"
                    dominantBaseline="middle"
                    fill="#e1e8e9"
                    fontSize="8"
                    fontFamily="monospace"
                  >
                    {Math.abs(angle)}
                  </text>

                  <text
                    x={halfWidth + 10}
                    y={y}
                    dominantBaseline="middle"
                    fill="#e1e8e9"
                    fontSize="8"
                    fontFamily="monospace"
                  >
                    {Math.abs(angle)}
                  </text>
                </g>
              );
            })}
          </g>
        </g>
      </g>

      {/* Fixed upper reference ticks */}
      {[-60, -30, 0, 30, 60].map((angle) => (
        <line
          key={angle}
          x1="0"
          y1="-82"
          x2="0"
          y2="-75"
          transform={`rotate(${angle})`}
          stroke="#dce5e8"
          strokeWidth="1"
        />
      ))}

      {/* Fixed top pointer */}
      <path
        d="M 0 -82 L -5 -72 L 5 -72 Z"
        fill="#c4f46b"
      />

      {/* Fixed rover reference marker */}
      <path
        d="M -40 0 H -22 V 17 M 40 0 H 22 V 17 M -5 0 H 5"
        fill="none"
        stroke="#c4f46b"
        strokeWidth="2"
        strokeLinecap="round"
      />
    </svg>
  );
}
