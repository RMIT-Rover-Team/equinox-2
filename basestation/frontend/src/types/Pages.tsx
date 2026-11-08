export const pages = ['Drive', 'Arm', 'Science', 'Excavator'] as const;
export type Page = (typeof pages)[number];
